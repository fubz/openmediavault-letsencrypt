<?php
/**
 * Copyright (c) 2015-2017 OpenMediaVault Plugin Developers
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

require_once('openmediavault/functions.inc');

class OMVRpcServiceLetsEncrypt extends \OMV\Rpc\ServiceAbstract
{
    public function getName()
    {
        return 'LetsEncrypt';
    }

    public function initialize()
    {
        $this->registerMethod('getSettings');
        $this->registerMethod('setSettings');
        $this->registerMethod('generateCertificate');
    }

    /**
     * Get the settings of the service.
     *
     * @param array $params The method paramaters.
     * @param array $context The context of the caller.
     * @return mixed
     *
     * @throws DatabaseException
     */
    public function getSettings($params, $context)
    {
        // Validate the RPC caller context.
        $this->validateMethodContext($context, ['role' => OMV_ROLE_ADMINISTRATOR]);
        // Get the configuration object.
        $db = \OMV\Config\Database::getInstance();
        $object = $db->get('conf.service.letsencrypt');
        return $object->getAssoc();
    }

    /**
     * Set the settings of the service.
     *
     * @param array $params The method paramaters.
     * @param array $context The context of the caller.
     * @return mixed
     *
     * @throws DatabaseException
     */
    public function setSettings($params, $context)
    {
        // Validate the RPC caller context.
        $this->validateMethodContext($context, ['role' => OMV_ROLE_ADMINISTRATOR]);
        // Validate the parameters of the RPC service method.
        $this->validateMethodParams($params, 'rpc.letsencrypt.setsettings');
        // Get the existing configuration object.
        $db = \OMV\Config\Database::getInstance();
        $object = $db->get('conf.service.letsencrypt');
        $object->setAssoc($params);
        $db->set($object);
        // Return the configuration object.
        return $object->getAssoc();
    }

    public function generateCertificate($params, $context)
    {
        // Validate the RPC caller context.
        $this->validateMethodContext($context, ['role' => OMV_ROLE_ADMINISTRATOR]);
        // Generate the letsencrypt certificates
        // Create the background process.
        return $this->execBgProc(function($bgStatusFilename, $bgOutputFilename)
            use ($context) {
                // Get the configuration object.
                $db = \OMV\Config\Database::getInstance();
                $object = $db->get('conf.service.letsencrypt');
                // execute certbot
                $cmdArgs = [];
                $cmdArgs[] = 'certonly';
                // if enabled, generate test parameter
                if (true == $object->get('test_cert')) {
                    $cmdArgs[] = '--test-cert';
                }
                $cmdArgs[] = '--webroot';
                $cmdArgs[] = sprintf('-w %s', $object->get('webroot'));
                $cmdArgs[] = '--text';
                $cmdArgs[] = '--keep-until-expiring';
                $cmdArgs[] = '--agree-tos';
                $cmdArgs[] = '--expand';
                $cmdArgs[] = sprintf('--email %s', $object->get('email'));
                //Generate domain parameters
                $domains = explode(',', $object->get('domain'));
                foreach ($domains as $domain) {
                    $cmdArgs[] = sprintf('-d %s', trim($domain));
                }
                $cmd = new \OMV\System\Process('/usr/bin/certbot', $cmdArgs);
                $cmdLine = $cmd->getCommandLine();
                $this->exec($cmdLine, $output, $bgOutputFilename);
                // Replace cert if not testing
                if (false == $object->get('test_cert')) {
                    if (is_uuid($object->get('certuuid'))) {
                        // Delete the old certificate to avoid duplicates
                        $oldcert = OMV\Rpc\Rpc::call('CertificateMgmt', 'delete', [
                            'uuid' => $object->get('certuuid')
                        ], $context);
                    }
                    $liveDir = "/etc/letsencrypt/live";
                    $keyFile = sprintf('%s/%s/privkey.pem', $liveDir, $domains[0]);
                    $crtFile = sprintf('%s/%s/fullchain.pem', $liveDir, $domains[0]);
                    if (file_exists($keyFile) && file_exists($crtFile)) {
                        // Read certificate and key content
                        // Import generated certificate.
                        $cert = OMV\Rpc\Rpc::call('CertificateMgmt', 'set', [
                            'uuid' => \OMV\Environment::get('OMV_CONFIGOBJECT_NEW_UUID'),
                            'certificate' => file_get_contents($crtFile),
                            'privatekey' => file_get_contents($keyFile),
                            'comment' => sprintf('LetsEncrypt - %s', $myDomains[0])
                        ], $context);
                        if (is_uuid($cert['uuid'])) {
                            $object->set('certuuid', $cert['uuid']);
                        }
                        // Resave the configuration data now with a new cert uuid
                        $new = OMV\Rpc\Rpc::call('LetsEncrypt', 'setSettings',
                            $object->getAssoc(), $context
                        );
                    }
                }
                // Create cron job if enabled
                $cmd = new \OMV\System\Process('omv-mkconf', 'letsencrypt');
                $cmd->setRedirect2to1();
                $cmd->execute();

                return $output;
            }
        );
    }
}

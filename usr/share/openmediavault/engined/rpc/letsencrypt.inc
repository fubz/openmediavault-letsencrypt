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

        $this->registerMethod('getDomainList');
        $this->registerMethod('getDomain');
        $this->registerMethod('setDomain');
        $this->registerMethod('deleteDomain');

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
         // Remove spaces from name
        $params['name'] = str_replace(' ', '_', $params['name']);
        // Get the existing configuration object.
        $db = \OMV\Config\Database::getInstance();
        $object = $db->get('conf.service.letsencrypt');
        $object->setAssoc($params);
        $db->set($object);
        // Return the configuration object.
        return $object->getAssoc();
    }

    public function getDomainList($params, $context)
    {
        // Validate the RPC caller context.
        $this->validateMethodContext($context, ['role' => OMV_ROLE_ADMINISTRATOR]);
        // Validate the parameters of the RPC service method.
        $this->validateMethodParams($params, 'rpc.common.getlist');
        // Get the configuration object.
        $db = \OMV\Config\Database::getInstance();
        $objects = $db->getAssoc('conf.service.letsencrypt.domain');
        // Filter the result.
        return $this->applyFilter($objects, $params['start'], $params['limit'],
            $params['sortfield'], $params['sortdir']);
    }

    public function getDomain($params, $context)
    {
        // Validate the RPC caller context.
        $this->validateMethodContext($context, ['role' => OMV_ROLE_ADMINISTRATOR]);
        // Validate the parameters of the RPC service method.
        $this->validateMethodParams($params, 'rpc.common.objectuuid');
        // Get the configuration object.
        $db = \OMV\Config\Database::getInstance();
        return $db->getAssoc('conf.service.letsencrypt.domain', $params['uuid']);
    }

    public function setDomain($params, $context)
    {
        // Validate the RPC caller context.
        $this->validateMethodContext($context, ['role' => OMV_ROLE_ADMINISTRATOR]);
        // Validate the parameters of the RPC service method.
        $this->validateMethodParams($params, 'rpc.letsencrypt.setdomain');
        // Prepare the configuration object.
        $object = new \OMV\Config\ConfigObject('conf.service.letsencrypt.domain');
        $object->setAssoc($params);
        // Set the configuration object.
        $isNew = $object->isNew();
        $db = \OMV\Config\Database::getInstance();
        if (TRUE === $isNew) {
            // Check uniqueness - web root
            $db->assertIsUnique($object, 'webroot');
        }
        $db->set($object);
        // Return the configuration object.
        return $object->getAssoc();
    }

    public function deleteDomain($params, $context)
    {
        // Validate the RPC caller context.
        $this->validateMethodContext($context, ['role' => OMV_ROLE_ADMINISTRATOR]);
        // Validate the parameters of the RPC service method.
        $this->validateMethodParams($params, 'rpc.common.objectuuid');
        // Delete the configuration object.
        $db = \OMV\Config\Database::getInstance();
        $object = $db->get('conf.service.letsencrypt.domain', $params['uuid']);
        $db->delete($object);
        // Return the deleted configuration object.
        return $object->getAssoc();
    }

    public function generateCertificate($params, $context)
    {
        // Validate the RPC caller context.
        $this->validateMethodContext($context, ['role' => OMV_ROLE_ADMINISTRATOR]);
        // Generate the letsencrypt certificates
        // Create the background process.
        return $this->execBgProc(function($bgStatusFilename, $bgOutputFilename)
            use ($params, $context) {
                // Get the configuration object.
                $db = \OMV\Config\Database::getInstance();
                $object = $db->get('conf.service.letsencrypt');
                $domains = $db->get('conf.service.letsencrypt.domain');
                $certuuid = $object->get('certuuid');
                // execute certbot
                $cmdArgs = [];
                if (($params['command'] == 'renew') && (is_uuid($certuuid))) {
                    $cmdArgs[] = 'renew';
                } else {
                    $cmdArgs[] = 'certonly';
                    $cmdArgs[] = '--non-interactive';
                    // if enabled, generate test parameter
                    if (true == $object->get('test_cert')) {
                        $cmdArgs[] = '--test-cert';
                    }
                    $cmdArgs[] = sprintf('--rsa-key-size %s', $object->get('keylength'));
                    $cmdArgs[] = '--text';
                    $cmdArgs[] = '--keep-until-expiring';
                    $cmdArgs[] = '--agree-tos';
                    $cmdArgs[] = '--allow-subset-of-names';
                    $cmdArgs[] = sprintf('--cert-name %s', $object->get('name'));
                    $cmdArgs[] = sprintf('--email %s', $object->get('email'));
                    $cmdArgs[] = '--webroot';
                    foreach ($domains as $domaink => &$domainv) {
                        $cmdArgs[] = sprintf('-w %s', $domainv->get('webroot'));
                        // convert domain list to array
                        $domains2 = explode(',', $domainv->get('domain'));
                        // generate domain parameters
                        foreach ($domains2 as $domain) {
                            $cmdArgs[] = sprintf('-d %s', trim($domain));
                        }
                    }
                }
                $cmd = new \OMV\System\Process('/usr/bin/certbot', $cmdArgs);
                $cmd->setRedirect2to1(true);
                $cmdLine = $cmd->getCommandLine();
                $this->writeBgProcOutput($bgOutputFilename, sprintf('Command: %s', $cmdLine));
                $this->writeBgProcOutput($bgOutputFilename, PHP_EOL.PHP_EOL);
                $this->exec($cmdLine, $output, $bgOutputFilename);
                // Replace cert if not testing
                if (false == $object->get('test_cert')) {
                    $liveDir = '/etc/letsencrypt/live';
                    $keyFile = sprintf('%s/%s/privkey.pem', $liveDir, $object->get('name'));
                    $crtFile = sprintf('%s/%s/fullchain.pem', $liveDir, $object->get('name'));
                    if (file_exists($keyFile) && file_exists($crtFile)) {
                        // use OMV_CONFIGOBJECT_NEW_UUID if certuuid is not a uuid
                        if (!is_uuid($certuuid)) {
                            $certuuid = \OMV\Environment::get('OMV_CONFIGOBJECT_NEW_UUID');
                        }
                        // Read certificate and key content
                        // Import generated certificate.
                        $cert = OMV\Rpc\Rpc::call('CertificateMgmt', 'set', [
                            'uuid' => $certuuid,
                            'certificate' => file_get_contents($crtFile),
                            'privatekey' => file_get_contents($keyFile),
                            'comment' => sprintf('LetsEncrypt - %s', $object->get('name'))
                        ], $context);
                        if (is_uuid($cert['uuid'])) {
                            $object->set('certuuid', $cert['uuid']);
                        }
                        // Resave the configuration data now with a new cert uuid
                        $new = OMV\Rpc\Rpc::call('LetsEncrypt', 'setSettings',
                            $object->getAssoc(), $context
                        );
                         // Apply changes immmediately
                        \OMV\Rpc\Rpc::call('Config', 'applyChanges', [
                            'modules' => [ 'certificatemgmt' ],
                            'force' => TRUE
                        ], $context);
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

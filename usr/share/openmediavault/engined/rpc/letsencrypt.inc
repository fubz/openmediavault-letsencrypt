<?php

/**
 * Copyright (c) 2015 OpenMediaVault Plugin Developers
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

require_once "openmediavault/error.inc";
require_once "openmediavault/notify.inc";
require_once "openmediavault/rpcservice.inc";

class OMVRpcServiceLetsEncrypt extends OMVRpcServiceAbstract
{
    /**
     * The main event message path.
     *
     * @var string
     */
    private $eventMessagePath = "org.openmediavault.services.letsencrypt";

    /**
     * Get the service configuration XPath.
     *
     * @return string
     */
    private function getXpath()
    {
        return "/config/services/letsencrypt";
    }

    private function getNetworkXPath()
    {
        return "/config/system/network";
    }
    /**
     * Get the name of the RPC service.
     *
     * @return string
     */
    public function getName()
    {
        return "LetsEncrypt";
    }

    /**
     * Initialize the RPC service.
     *
     * @return void
     */
    public function initialize()
    {
        $this->registerMethod("getSettings");
        $this->registerMethod("setSettings");
        $this->registerMethod("generateCertificate");
    }

    /**
     * Get the settings of the service.
     *
     * @param array $params The method paramaters.
     * @param array $context The context of the caller.
     * @return mixed
     *
     * @throws OMVException
     */
    public function getSettings($params, $context)
    {
        global $xmlConfig;

        // Validate the RPC caller context.
        $this->validateMethodContext($context, array(
            "role" => OMV_ROLE_ADMINISTRATOR
        ));

        // Get the configuration object.
        $object = $xmlConfig->get($this->getXpath());

        if (is_null($object)) {
            throw new OMVException(
                OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
                $this->getXpath()
            );
        }

        //$get system settings
        $network = $xmlConfig->get($this->getNetworkXPath());
        $settingsObject["domain"] = $network["domainname"];

        // Modify the result data.
        if ( strlen($object["domain"]) > 0) {
            $settingsObject["domain"] = $object["domain"];
        }

        $settingsObject["enable"] = boolval($object["enable"]);
        $settingsObject["email"] = $object["email"];
        $settingsObject["cn"] = $object["cn"];
        $settingsObject["ou"] = $object["ou"];
        $settingsObject["o"] = $object["o"];
        $settingsObject["l"] = $object["l"];
        $settingsObject["st"] = $object["st"];
        $settingsObject["c"] = $object["c"];



        return $settingsObject;
    }

    /**
     * Set the settings of the service.
     *
     * @param array $params The method paramaters.
     * @param array $context The context of the caller.
     * @return mixed
     *
     * @throws OMVException
     */
    public function setSettings($params, $context)
    {
        global $xmlConfig;

        // Validate the RPC caller context.
        $this->validateMethodContext($context, array(
            "role" => OMV_ROLE_ADMINISTRATOR
        ));

        // Validate the parameters of the RPC service method.
        $this->validateMethodParams(
            $params,'{
                "type": "object",
                "properties": {
                    "enable": {"type": "boolean"},
                    "domain":{"type":"string","minLength":1},
                    "c":{"type":"string","maxLength":2},
                    "st":{"type":"string"},
                    "l":{"type":"string"},
                    "o":{"type":"string"},
                    "ou":{"type":"string"},
                    "cn":{"type":"string","minLength":1},
                    "email":{'.$GLOBALS['OMV_JSONSCHEMA_EMAIL'].'}
                }
            }'
        );

        //Do Work here!





        //NO BACKGROUND
//        $cmd = sprintf("export LANG=C; sh /opt/letsencrypt/letsencrypt-auto certonly --standalone --text --keep-until-expiring --agree-tos --email \"%s\" -d \"%s\" > /home/fubz/test.txt",
//            $params['email'], $params['domain']);
//        $cmd .= " 2>&1";
//        if (0 !== $this->exec($cmd, $output)) {
//            throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
//                $cmd, implode("\n", $output));
//        }
        //END NO BACKGROUND








        // Prepare the configuration data.
        $object = array(
            "enable" => boolval($params["enable"]),
            "domain" => $params['domain'],
            "email" => $params['email'],
            "cn" => $params['cn'],
            "ou" => $params['ou'],
            "o" => $params['o'],
            "l" => $params['l'],
            "st" => $params['st'],
            "c" => $params['c']
        );

        // Set the configuration object.
        if (false === $xmlConfig->replace($this->getXpath(), $object)) {
            throw new OMVException(
                OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED,
                $this->getXpath()
            );
        }

        // Notify configuration changes.
        $dispatcher = &OMVNotifyDispatcher::getInstance();
        $dispatcher->notify(
            OMV_NOTIFY_MODIFY,
            $this->eventMessagePath,
            $object
        );

        return $object;
    }

    public function generateCertificate($params, $context)
    {
        global $xmlConfig;

        // Validate the RPC caller context.
        $this->validateMethodContext($context, array(
            "role" => OMV_ROLE_ADMINISTRATOR
        ));

        // Check if the module is marked as dirty. This is an indication
        // that configuration has not been saved
        if ($this->isModuleDirty("certificatemgmt"))
            throw new OMVException(OMVErrorMsg::E_CONFIG_DIRTY);

        // Get the configuration object.
        $params = $xmlConfig->get($this->getXpath());

        //From rpc/certificatemgmt.inc
        // Generate the certificate subject string.
        $rdnfields = array("c", "st", "l", "o", "ou", "cn");
        $subject = "";
        foreach ($rdnfields as $rdnfieldk => $rdnfieldv) {
            if (empty($params[$rdnfieldv]))
                continue;
            // Escape RDN attribute value, see
            // http://www-03.ibm.com/systems/i/software/ldap/underdn.html
            // http://msdn.microsoft.com/en-us/library/windows/desktop/aa366101(v=vs.85).aspx
            $replace = array();
            foreach (array(',','+','<','>',';','\r','=','#','/') as $v) {
                $replace[$v] = sprintf("\\%s", strtoupper(str_pad(dechex(
                    ord($v)), 2, "0")));
            }
            $params[$rdnfieldv] = strtr($params[$rdnfieldv], $replace);
            $params[$rdnfieldv] = addcslashes($params[$rdnfieldv], '"\\');
            // Append to subject.
            switch ($rdnfieldv) {
                case "cn":
                    // Append the email address.
                    if (!empty($params['email'])) {
                        $params[$rdnfieldv] = sprintf("%s/emailAddress=%s",
                            $params[$rdnfieldv], $params['email']);
                    }
                default:
                    $subject .= sprintf("/%s=%s", strtoupper($rdnfieldv),
                        $params[$rdnfieldv]);
            }
        }


        //Generate the letsencrypt certificates
        // Create a background process.
        $bgStatusFilename = $this->createBgProcStatus();
        $pid = $this->fork();
        if ($pid > 0)    // Parent process.
        {
            $this->initializeBgProcStatus($bgStatusFilename, $pid);
            return $bgStatusFilename;
        }

        // Child process.
        try {
            $bgOutputFilename = $this->createBgProcOutput();
            $this->updateBgProcStatus($bgStatusFilename, "letsencryptout", $bgOutputFilename);


            //$cmd = sprintf("export LANG=C; service nginx stop && sh /opt/letsencrypt/letsencrypt-auto certonly --standalone --text --keep-until-expiring --agree-tos --email \"%s\" -d \"%s\" > /home/fubz/test.txt && service nginx start",
            $cmd = sprintf("sh /opt/letsencrypt/letsencrypt-auto certonly --webroot -w /var/www/openmediavault/ --text --keep-until-expiring --agree-tos --email \"%s\" -d \"%s\" > /home/fubz/test.txt",
                $params['email'], $params['domain']);

            $this->exec($cmd, $output, $bgOutputFilename);
            $this->finalizeBgProcStatus($bgStatusFilename, $output);


            // Read certificate and key content
            $keydata = file_get_contents(sprintf("/etc/letsencrypt/live/%s/privkey.pem", $params['domain']));
            $crtdata = file_get_contents(sprintf("/etc/letsencrypt/live/%s/fullchain.pem", $params['domain']));

            // Finally import generated certificate.
            $rpcServiceMgr = &OMVRpcServiceMgr::getInstance();
            $rpcService = $rpcServiceMgr->getService("CertificateMgmt");

            $return = $rpcService->callMethod("set", array(
                "uuid" => $GLOBALS['OMV_UUID_UNDEFINED'],
                "certificate" => $crtdata,
                "privatekey" => $keydata,
                "comment" => !empty($subject) ? $subject : ""
            ), $context);


            //$log = sprintf("[%s] omv-snapraid-gui: INFO: %s - Finished\r\n", date("Y-m-d H:i:s"), strtoupper($params['command']));
            //file_put_contents($this->getLogFile(), $log, FILE_APPEND);
            exit(0);
        } catch(Exception $e) {
            $this->finalizeBgProcStatus($bgStatusFilename, "", $e);
            //$log = sprintf("[%s] omv-snapraid-gui: ERROR: %s - Error!\r\n", date("Y-m-d H:i:s"), strtoupper($params['command']));
            //file_put_contents($this->getLogFile(), $log, FILE_APPEND);
            exit(1);
        }

    }

    /**
     * Get statistics, e.g. who's online.
     * @param params The method parameters.
     * @param context The context of the caller.
     * @return A string containing the requested statistics.
     * @throw E_CONFIG_GET_OBJECT_FAILED
     * @throw E_EXEC_FAILED
     */
    public function getStats($params, $context) {
        global $xmlConfig;
        // Validate the RPC caller context.
        $this->validateMethodContext($context, array(
            "role" => OMV_ROLE_ADMINISTRATOR
        ));
        // Is service enabled?
        $xpath = "//services/letsencrypt";
        $object = $xmlConfig->get($xpath);
        if(is_null($object)) {
            throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
                $xpath);
        }
        if (!boolvalEx($object['enable'])) {
            $stats = gettext("Service disabled");
        } else {
            // Get users currently logged in.
//            $stats = sprintf("%s:\n", gettext("Currently logged in users"));
//            $cmd = "export LANG=C; who -u 2>&1";
//            if(0 !== $this->exec($cmd, $output)) {
//                throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
//                    $cmd, implode("\n", $output));
//            }
//            $stats .= empty($output) ? gettext("No users connected") :
//                implode("\n", $output);
//            // Get connections.
//            $cmd = sprintf("export LANG=C; lsof -n -s TCP:ESTABLISHED ".
//                "-i :%d 2>&1 || true", $object['port']);
//            if(0 !== $this->exec($cmd, $output)) {
//                throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
//                    $cmd, implode("\n", $output));
//            }
//            $stats .= sprintf("\n\n%s:\n", gettext("Connections"));
//            $stats .= empty($output) ? gettext("No connections") :
//                implode("\n", $output);
            $stats = gettext("Hello world!");
        }
        return $stats;
    }



    }

// Register the RPC service.
$rpcServiceMgr = &OMVRpcServiceMgr::getInstance();
$rpcServiceMgr->registerService(new OMVRpcServiceLetsEncrypt());

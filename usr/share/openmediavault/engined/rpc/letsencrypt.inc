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
require_once "openmediavault/functions.inc";
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
        $settingsObject["test_cert"] = boolval($object["test_cert"]);
        $settingsObject["email"] = $object["email"];
        $settingsObject["webroot"] = $object["webroot"];
        $settingsObject["certuuid"] = $object["certuuid"];
        $settingsObject["cron_uuid"] = $object["cron_uuid"];

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
                    "test_cert": {"type": "boolean"},
                    "domain":{"type":"string","minLength":1},
                    "email":{'.$GLOBALS['OMV_JSONSCHEMA_EMAIL'].'},
                    "webroot":{"type":"string","minLength":1},
                    "certuuid":{"type":"string"},
                    "cron_uuid":{"type":"string"}
                }
            }'
        );



        #If 'enable' is set, a cron job is required
        //Get reference to the Certificate RPC
        $rpcServiceMgr = &OMVRpcServiceMgr::getInstance();
        $rpcService = $rpcServiceMgr->getService("cron");

        if($params['enable'] && false == is_uuid($params['cron_uuid'] && false == $params['test_cert'])) {
            $cron_minute=rand(0, 59);
            $cron_hour=rand(0, 23);

            // Finally import generated certificate.
            $return = $rpcService->callMethod("set", array(
                "uuid"          => $GLOBALS['OMV_UUID_UNDEFINED'],
                "enable"        => true,
                "execution"     => "monthly",
                "sendemail"     => false,
                "type"          => "userdefined",
                "minute"        => strval($cron_minute),
                "everynminute"  => false,
                "hour"          => strval($cron_hour),
                "everynhour"    => false,
                "dayofmonth"    => "*",
                "everyndayofmonth" => false,
                "month"         => "*",
                "dayofweek"     => "*",
                "username"      => "root",
                "command"       => "omv-letsencrypt",
                "comment"       => "Keeps certificate generate by Lets Encrypt plugin up to date.  Do not delete here, disable from Lets Encrypt plugin."
            ), $context);

            $params['cron_uuid'] = $return['uuid'];
        } else if (false == $params['enable'] && is_uuid($params['cron_uuid'])  ) {
            // Finally import generated certificate.
            $return = $rpcService->callMethod("delete", array(
                "uuid"          => $params['cron_uuid']
            ), $context);

            $params['cron_uuid'] = "";
        }


        // Prepare the configuration data.
        $object = array(
            "enable" => boolval($params["enable"]),
            "test_cert" => boolval($params["test_cert"]),
            "domain" => $params['domain'],
            "email" => $params['email'],
            "webroot" => $params['webroot'],
            "certuuid" => $params['certuuid'],
            "cron_uuid" => $params['cron_uuid']
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
        if ($this->isModuleDirty("letsencrypt"))
            throw new OMVException(OMVErrorMsg::E_CONFIG_DIRTY);

        // Get the configuration object.
        $params = $xmlConfig->get($this->getXpath());


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
            $this->updateBgProcStatus($bgStatusFilename, "outputfilename", $bgOutputFilename);

            //Generate test parameter
            $testCert = "";
            if ($params['test_cert']) {
                $testCert = "--test-cert " ;
            }

            //Generate domain parameters
            $myDomains = explode(',', $params['domain']);
            $domainList = "";
            foreach($myDomains as $aDomain){
                $domainList = $domainList." -d ".trim($aDomain);
            }

            $cmd = "sh /opt/letsencrypt/letsencrypt-auto certonly ".$testCert."--webroot -w ".$params['webroot']." --text --keep-until-expiring --agree-tos --expand --email ".$params['email']." ".$domainList." 2>&1";

            if (0 !== $this->exec($cmd, $output, $bgOutputFilename)) {
                throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
                    $cmd, implode("\n", $output));
            }
            //$exception = "Success";
            $this->finalizeBgProcStatus($bgStatusFilename, $output); //$exception

            if (false == $params['test_cert']) {
                //Get reference to the Certificate RPC
                $rpcServiceMgr = &OMVRpcServiceMgr::getInstance();
                $rpcService = $rpcServiceMgr->getService("CertificateMgmt");

                if (is_uuid($params['certuuid'])) {
                    // Delete the old certificate to avoid duplicates
                    $return = $rpcService->callMethod("delete", array(
                        "uuid" => $params['certuuid']
                    ), $context);
                }

                // Read certificate and key content
                $keydata = file_get_contents(sprintf("/etc/letsencrypt/live/%s/privkey.pem", $myDomains[0]));
                $crtdata = file_get_contents(sprintf("/etc/letsencrypt/live/%s/fullchain.pem", $myDomains[0]));

                // Finally import generated certificate.
                $return = $rpcService->callMethod("set", array(
                    "uuid" => $GLOBALS['OMV_UUID_UNDEFINED'],
                    "certificate" => $crtdata,
                    "privatekey" => $keydata,
                    "comment" => "LetsEncrypt - ".$myDomains[0]
                ), $context);


                //Resave the configuration data now with a new cert uuid
                $object = array(
                    "enable" => boolval($params["enable"]),
                    "test_cert" => boolval($params["test_cert"]),
                    "domain" => $params['domain'],
                    "email" => $params['email'],
                    "webroot" => $params['webroot'],
                    "certuuid" => $return['uuid'],
                    "cron_uuid" => $params['cron_uuid']
                );

                $xmlConfig->replace($this->getXpath(), $object);
            }


            exit(0);
        } catch(Exception $e) {
            $this->finalizeBgProcStatus($bgStatusFilename, "", $e);
            exit(1);
        }

    }

}

// Register the RPC service.
$rpcServiceMgr = &OMVRpcServiceMgr::getInstance();
$rpcServiceMgr->registerService(new OMVRpcServiceLetsEncrypt());


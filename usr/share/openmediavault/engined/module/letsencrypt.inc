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

require_once "openmediavault/config.inc";
require_once "openmediavault/error.inc";
require_once "openmediavault/initscript.inc";
require_once "openmediavault/module.inc";
require_once "openmediavault/notify.inc";
require_once "openmediavault/log.inc";

class OMVModuleLetsEncrypt extends OMVModuleServiceAbstract implements
    OMVINotifyListener
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

    /**
     * Get the module name.
     *
     * @return string
     */
    public function getName()
    {
        return "letsencrypt";
    }

    /**
     * Get the status of the service.
     *
     * @return array
     *
     * @throws OMVException
     */
    public function getStatus()
    {
        global $xmlConfig;

        $object = $xmlConfig->get($this->getXpath());

        if (is_null($object)) {
            throw new OMVException(
                OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
                $this->getXpath()
            );
        }

        $initd = new OMVSysVInitScript($this->getName());

        return array(
            "name" => $this->getName(),
            "title" => gettext("LetsEncrypt"),
            "enabled" => boolval($object["enable"]),
            "running" => $initd->status() == 0
        );
    }

    /**
     * Create and update the configuration.
     *
     * @return void
     *
     * @throws OMVException
     */
    public function applyConfig()
    {
        $cmd = sprintf("export LANG=C; omv-mkconf %s 2>&1", $this->getName());

        if (0 !== $this->exec($cmd, $output)) {
            throw new OMVException(
                OMVErrorMsg::E_EXEC_FAILED,
                $cmd,
                implode(PHP_EOL, $output)
            );
        }
    }

    /**
     * Start the managed service.
     *
     * @return void
     *
     * @throws OMVException
     */
    public function startService()
    {
        global $xmlConfig;

        $object = $xmlConfig->get($this->getXpath());

        if (is_null($object)) {
            throw new OMVException(
                OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
                $this->getXpath()
            );
        }

        if (!$object["enable"]) {
            return;
        }

        $initd = new OMVSysVInitScript($this->getName(), true);
        //$initd->exec();
    }

    /**
     * Stop the managed service.
     *
     * @return void
     */
    public function stopService()
    {
        $initd = new OMVSysVInitScript($this->getName());
        //$initd->exec();
    }

    /**
     * Bind listeners.
     *
     * @return void
     */
    public function bindListeners(OMVNotifyDispatcher $dispatcher)
    {
        $dispatcher->addListener(
            OMV_NOTIFY_CREATE | OMV_NOTIFY_MODIFY | OMV_NOTIFY_DELETE,
            $this->eventMessagePath,
            array($this, "setDirty")
        );
    }
}

// Register the module.
$moduleMgr = &OMVModuleMgr::getInstance();
$moduleMgr->registerModule(new OMVModuleLetsEncrypt());


// Register new logfile type
//Sample:
//2016-01-17 19:34:29,266:DEBUG:letsencrypt.cli:Picked account: <Account(eb49da97b040939e9439a6a1c123294c)>
OMVLogFile::registerType("letsencrypt", array(
    "filename" => "letsencrypt",
    "filepath" => "/var/log/letsencrypt/letsencrypt.log",
    "regex"    => "/^(.+,\d{3}):(\w+):([\w\.]+):([[:print:]]+)$/",
    "columns"  => array(
        "ts" => array(
            "index" => 1,
            "func"  => create_function('$v', 'return strpdate($v,"Y-m-d G:i:s,u");')
        ),
        "date" => array(
            "index" => 1,
            "func"  => create_function('$v', 'return strftime("%c", strpdate($v,"Y-m-d G:i:s,u"));')
        ),
        "component" => 3,
        "level"     => 2,
        "message"   => 4
    )
));

/*
 *
 */

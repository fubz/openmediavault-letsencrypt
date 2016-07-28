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


class OMVModuleLetsEncrypt extends \OMV\Engine\Module\ServiceAbstract
  implements \OMV\Engine\Notify\IListener
{
    /**
     * The main event message path.
     *
     * @var string
     */
    private $eventMessagePath = "org.openmediavault.services.letsencrypt";

    private $dataModelPath = 'config.service.letsencrypt';

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

        $database = \OMV\Config\Database::getInstance();
        $object = $database->get($this->dataModelPath);

        $systemCtl = new \OMV\Sysystem\SystemCtl($this->getName());

        return [
            "name" => $this->getName(),
            "title" => gettext("LetsEncrypt"),
            "enabled" => $object->get("enable"),
            "running" => $systemCtl->isActive()
        ];
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
        $cmd = new \OMV\System\Process("omv-mkconf", $this->getName());
        $cmd->setRedirect2to1();
        $cmd->execute();
    }

    /**
     * Bind listeners.
     *
     * @return void
     */
    public function bindListeners(\OMV\Engine\Notify\Dispatcher $dispatcher)
    {
        $dispatcher->addListener(
            OMV_NOTIFY_CREATE | OMV_NOTIFY_MODIFY | OMV_NOTIFY_DELETE,
            $this->eventMessagePath,
            [$this, "setDirty"]
        );
    }
}


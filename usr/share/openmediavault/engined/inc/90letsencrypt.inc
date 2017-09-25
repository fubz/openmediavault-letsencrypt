<?php

require_once("openmediavault/functions.inc");

// Register new logfile type
// Sample:
// 2016-01-17 19:34:29,266:DEBUG:letsencrypt.cli:Picked account: <Account(eb49da97b040939e9439a6a1c123294c)>
\OMV\System\LogFileSpec::registerSpecification("letsencrypt", [
    "filename" => "letsencrypt",
    "filepath" => "/var/log/letsencrypt/letsencrypt.log",
    "regex"    => "/^(.+,\d{3}):(\w+):([\w\.]+):([[:print:]]+)$/",
    "columns"  => [
        "ts" => [
            "index" => 1,
            "func"  => create_function('$v', 'return strpdate($v . " UTC","Y-m-d G:i:s,u T");')
        ],
        "date" => [
            "index" => 1,
            "func"  => create_function('$v', 'return strftime("%c", strpdate($v . " UTC","Y-m-d G:i:s,u T"));')
        ],
        "component" => 3,
        "level"     => 2,
        "message"   => 4
    ]
]);


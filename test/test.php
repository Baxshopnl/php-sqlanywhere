<?php

$loaded = extension_loaded('sqlanywhere');

if ($loaded) {
    echo 'ext-sqlanywere loaded successfully' . PHP_EOL;
}

exit($loaded ? 0 : 1);
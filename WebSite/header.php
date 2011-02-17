<html xmlns="http://www.w3.org/1999/xhtml">
    <head>
    <meta http-equiv="content-type" content="text/html; charset=utf-8" />
    <title>Ditto clipboard manager</title>
    <meta name="keywords" content="" />
    <meta name="description" content="" />
    <link href="default.css" rel="stylesheet" type="text/css" />
    </head>
    <body>
        <div id="wrapper">
            <div id="logo">
                <h1><a href="index.php">Ditto</a></h1>
                <h2><a href="https://sourceforge.net/projects/ditto-cp/files/Ditto/3.17.0.17/DittoSetup_3_17_0_17.exe/download"> &raquo;&nbsp;&nbsp;&nbsp;Clipboard Manager &raquo; 3.17.0.17</a></h2>
            </div>
            <div id="header">
                <div id="menu">
                    <ul>
                        <li class=<?php GetActiveFlag('Index', $focusTab)?>><a href="index.php">Homepage</a></li>
                        <li class=<?php GetActiveFlag('Translate', $focusTab)?>><a href="http://sourceforge.net/apps/trac/ditto-cp/wiki/Translate">Translate</a></li>
                        <li class=<?php GetActiveFlag('Themes', $focusTab)?>><a href="http://sourceforge.net/apps/trac/ditto-cp/wiki/Current%20Themes">Themes</a></li>
                        <li><a href="http://sourceforge.net/apps/trac/ditto-cp/wiki">Wiki</a></li>
                        <li><a href="http://ditto-cp.sourceforge.net/Help">Help</a></li>
                        <li class=<?php GetActiveFlag('History', $focusTab)?>><a href="#Change History">Change History</a></li>
                        <li class="last"><a href="http://sourceforge.net/forum/?group_id=84084">Forums</a></li>
						
						<li class="last"><b><a href="http://sourceforge.net/project/project_donations.php?group_id=84084">Donote</a></b></li>
                    </ul>
                </div>
            </div>
        </div>
    </body>
</html>


<?php

function GetActiveFlag($item, $page) 
{
	if($item == $page)
	{
		print 'current_page_item';
	}
}

?> 
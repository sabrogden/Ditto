<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.1//EN" "http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<meta http-equiv="content-type" content="text/html; charset=utf-8" />
<title>Ditto clipboard manager</title>
<meta name="keywords" content="Ditto, clipboard, manager" />
<meta name="description" content="" />
<link href="default.css" rel="stylesheet" type="text/css" />
<?php $focusTab = "Index"; Include("header.php");?>
</head>

<body>       
        
    <div id="page">
        <div id="content">
            <div class="post">
              <div class="entry">
                  <p>Ditto is an extension to the standard windows clipboard.  It saves each item placed on the clipboard allowing you access to any of those items at a later time.  Ditto allows you to save any type of information that can be put on the clipboard, text, images, html, custom formats, .....</p>
              </div>
            </div>

			<a name="Project Reviews"></a>
			<div class="post">
				<h1 class="title"><a href="http://sourceforge.net/projects/ditto-cp/reviews/">Project Reviews</a></h1>
				<script language="JavaScript" src="http://itde.vccs.edu/rss2js/feed2js.php?src=http%3A%2F%2Fsourceforge.net%2Fprojects%2Fditto-cp%2Freviews_feed.rss&chan=n&num=5&desc=1&date=n&targ=n" type="text/javascript"></script>
				<noscript>
				<a href="http://itde.vccs.edu/rss2js/feed2js.php?src=http%3A%2F%2Fsourceforge.net%2Fprojects%2Fditto-cp%2Freviews_feed.rss&chan=n&num=10&desc=1&date=n&targ=n&html=y">View RSS feed</a>
				</noscript>
			</div>
		  
			<div class="post">
                <h1 class="title">Features</h1>
                <div class="entry">
                    <ul>
                      <li>Easy to use interface</li>
                      <li>Search and paste previous copy entries</li>
                      <li><b>Keep multiple computer's clipboards in sync</b></li>
                      <li>Data is encrypted when sent over the network</li>
                      <li>Accessed from tray icon or global hot key</li>
                      <li>Select entry by double click, enter key or drag drop</li>
                      <li>Paste into any window that excepts standard copy/paste entries</li>
                      <li>Display thumbnail of copied images in list</li>
                      <li>Full Unicode support(display foreign characters)</li>
                      <li>UTF-8 support for language files(create language files in any language)</li>
                      <li>Uses sqlite database (<a href="http://www.sqlite.org">www.sqlite.org</a>)</li>
                    </ul>
              </div>
			</div>
 
          <div class="post">
                <h1 class="title">Why not use built in Copy bins in Office or VS.Net</h1>
                <div class="entry">
                    <ul>
                         <li>VS.Net only collects pastes from inside Visual studio</li>
						 <li>No way to paste to external app</li>
                         <li>Can't search past clips</li>
                         <li>Limited storage of clips</li>
                         <li>Clips do not persist after closing Visual Studio</li>
                    </ul>
              </div>
          </div>
          
            <a name="Change History"></a>
			
			<div class="post">
                <h1 class="title">3.18.20.0 12-23-11</h1>
                <div class="entry">
                    <ul>
						<li>64bit build
						<li>Clip shortcut keys can be global (you don't need to open ditto for your clip shortcut keys to work).  New check box on the clip properties window.</li>
						<li>New shortcut key window (right click on icon - Global hot keys) so you can see what clips are available globally and was ditto able to register the shortcut key.</li>
						<li>Updated icon</li>
						<li>Removed draw rich text option (was finding this didn't work with office 2010 rtf text)</li>
						<li>Fixed crash when viewing images through F3</li>
						<li>Portable setting is now based off of the file "portable" not a setting in the config file</li>
						<li>Backup the db when running an update script</li>
						<li>Fixed crash when automatically closing ditto's window</li>
						<li>Fixed issue with displaying chinease language</li>
                    </ul>
              </div>
              <div class="meta">
				<p class="byline"><a href="changeHistory.php" class="more">Full Change History</a></p>
			  </div>
          </div>
		  
			<div class="post">
                <h1 class="title">3.17.00.17 12-23-10</h1>
                <div class="entry">
                    <ul>
						<li>Removed named paste, named paste items can be searched by entering /q text</li>
						<li>Added add-in to set/remove read only flag on clip containing cf_hdrop items or just text of file names</li>
						<li>Added add-in to remove all line feeds then paste the clip</li>
						<li>Fixed issue with ditto taking focus back, happened when always on-top was selected</li>
						<li>Reverted to previous method for setting the focus</li>
						<li>Fixed issue where 'v' was pasted instead of the actual clip</li>
						<li>Changed default method for tracking focus to polling</li>
						<li>Sped up clip deletes, delete of the large clipboard data now happens in the background</li>
						<li>Sped up filling of the list, only items in view are loaded</li>
						<li>Save connected to the clipboard state to config settings</li>
						<li>Search full cf_unicode clip data with /f in the search text ex) /f text</li>
						<li>Include correct version of mfc and c++ runtime files</li>
						<li>Removed auto update feature</li>
                    </ul>
              </div>
              <div class="meta">
				<p class="byline"><a href="changeHistory.php" class="more">Full Change History</a></p>
			  </div>
          </div>
			
			
			<div class="post">
                <h1 class="title">3.16.8.0 08-16-09</h1>
                <div class="entry">
                    <ul>
						<li>Added DittoUtil Addin, adds the ability to paste any clip type as text</li>
						<li>Fixed SetFocus fix in ActivateTarget -- needed AttachThreadInput</li>
						<li>Added the ability to create add-ins, called before an item is pasted</li> 
						<li>Fixed issue with getting the currently focused window when not using the hook dll</li> 
						<li>Updated italiano language file</li> 
						<li>Added option to paste from hot key, press multiple times to move the selection, release the modifer key (control, shift, alt) to paste</li> 
						<li>Fixed word wrap option to reload correctly</li> 
						<li>Fixed issues with loading cut copy buffer 3 correctly from config on restart</li> 
						<li>Check if key is up before sending key up command. This was causing problems if an app is listening to global key up commands</li> 
						<li>Updated to sqlite version 3.16.10</li> 
                    </ul>
              </div>
              <div class="meta">
				<p class="byline"><a href="changeHistory.php" class="more">Full Change History</a></p>
			  </div>
          </div>
		  
            <div class="post">
                <h1 class="title">3.16.5.0 03-23-08</h1>
                <div class="entry">
                    <ul>
						<li>Fixed SetFocus fix in ActivateTarget -- needed AttachThreadInput -- wait for window to gain focus</li>
						<li>Added the ability to create add-ins, called before an item is pasted</li>
						<li>Fixed issue with getting the currently focused window when not using the hook dll</li>
						<li>Updated italiano language file</li>
						<li>Added option to paste from hot key, press multiple times to move the selection, release the modifer key (control, shift, alt) to paste</li>
						<li>Fixed word wrap option to reload correctly</li>
						<li>Fixed issues with loading cut copy buffer 3 correctly from config on restart</li>
						<li>Check if key is up before sending key up command.  This was causing problems if an app is listening to global key up commands</li>
						<li>Updated to sqlite version 3.16.10</li>
                    </ul>
              </div>
              <div class="meta">
				<p class="byline"><a href="changeHistory.php" class="more">Full Change History</a></p>
			  </div>
          </div>
			
			<div class="post">
                <h1 class="title">3.15.4.0 01-16-08</h1>
                <div class="entry">
                    <ul>
                      <li>Fixed empty directory from being created in application data in stand alone version</li>
                      <li>Added themes (http://ditto-cp.wiki.sourceforge.net/Themes)</li>
                      <li>Updated to latest sqlite db version</li>
                      <li>When creating a new db auto vacuum is set. Or when doing a compact and repair.</li>
                    </ul>
              </div>
              <div class="meta">
				<p class="byline"><a href="changeHistory.php" class="more">Full Change History</a></p>
			  </div>
          </div>

               <!-- Start of StatCounter Code -->
<script type="text/javascript">
var sc_project=4303469; 
var sc_invisible=0; 
var sc_partition=54; 
var sc_click_stat=1; 
var sc_security="b3f57099"; 
</script>

<script type="text/javascript" src="http://www.statcounter.com/counter/counter.js"></script><noscript><div class="statcounter"><a title="site stats" href="http://www.statcounter.com/" target="_blank"><img class="statcounter" src="http://c.statcounter.com/4303469/0/b3f57099/0/" alt="site stats" ></a></div></noscript>
<!-- End of StatCounter Code --><br><a href="http://my.statcounter.com/project/standard/stats.php?project_id=4303469&guest=1">View My Stats</a>
            
        </div>
        
       <?php Include("rightSide.php");?>
       

	</div>
        

    </div>

</body>
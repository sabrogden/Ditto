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
            
            <div class="post">
                <h1 class="title">3.15.1.0 8-19-07</h1>
                <div class="entry">
                    <ul>
                      <li>Custom Key strokes per application (http://ditto-cp.wiki.sourceforge.net/Custom+key+strokes)</li>
                      <li>Added option to hide friends, via registry setting only (http://ditto-cp.wiki.sourceforge.net/Disable+Friends)</li>
                      <li>Change some default values on new installs, limit copies to 500, disable friends, show at caret</li>
                      <li>Save word wrap option</li>
                      <li>Editor hot keys, added keys for undo, redo, wrap, bold, underline</li>
                    </ul>
              </div>
              <div class="meta">
				<p class="byline"><a href="changeHistory.php" class="more">Full Change History</a></p>
			  </div>
          </div>
            
            <div class="post">
                <h1 class="title">3.14.0.0 2-18-06</h1>
                <div class="entry">
                    <ul>
                        <li>Added new feature, "Ditto copy buffers" Options - Copy buffers. Can assign short cut keys for extra copy buffers. These copy buffers work just like Ctrl-C and Ctrl-V. Things can be copied and pasted without affecting the standard clipboard data.</li>
                        <li>Updated to latest sqlite db version. This fixed the problem with foreign characters in the current user's name.</li>
                        <li>Added CF_HDROP (copied files), Rich Text, Html Format to multi-paste support. (will group those types on multi-paste)</li>
                        <li>Added option for what separater is put between multi-pasted clips. (Options - General)</li>
                        <li>Added "Word Wrap" setting in clip editor</li>
                        <li>When searching in a group only search the clips in that group.</li>
                        <li>Hide apply button in options. The button didn't do anything.</li>
                        <li>Don't underline the character following an ampersand(&) when displaying clip text.</li>
                    </ul>
              </div>
              <div class="meta">
				<p class="byline"><a href="changeHistory.php" class="more">Full Change History</a></p>
			  </div>
            </div>
            
            <div class="post">
                <h1 class="title">3.9.0.0 8-03-06</h1>
                <div class="entry">
                    <ul>
	                    <li>Fixed issue that could cause Ditto to not disconnect from the clipboard</li>
                    </ul>
              </div>
              <div class="meta">
				<p class="byline"><a href="changeHistory.php" class="more">Full Change History</a></p>
			  </div>
            </div>
            
            <div class="post">
                <h1 class="title">3.8.0.0 7-27-06</h1>
                <div class="entry">
                    <ul>
	                    <li>Added the command line parameters "-disconnect" and "-connect" to control if ditto is connected to the clipboard or not</li>
						<li>Added option to output debug strings(F5) to file(Ctrl-F5) or debugview.exe (F5/Ctrl - F5 in quick paste window)</li>
						<li>Fixed error that caused some foreign characters as line feeds in Ditto</li>
                    </ul>
              </div>
              <div class="meta">
				<p class="byline"><a href="changeHistory.php" class="more">Full Change History</a></p>
			  </div>
            </div>
            
            <div class="post">
                <h1 class="title">3.6.0.0 6-06-06</h1>
                <div class="entry">
                    <ul>
                    	<li>Added the ability to edit clips</li>
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
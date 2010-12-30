<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.1//EN" "http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<meta http-equiv="content-type" content="text/html; charset=utf-8" />
<title>Ditto clipboard extender</title>
<meta name="keywords" content="" />
<meta name="description" content="" />
<link href="default.css" rel="stylesheet" type="text/css" />
<?php $focusTab = "History"; Include("header.php");?>
</head>

<body>       
        
    <div id="page">
        <div id="content">
          
            <a name="Change History"></a>
			
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
                <h1 class="title">3.16.5.0 03-23-098</h1>
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
            </div>
            
            <div class="post">
                <h1 class="title">3.9.0.0 8-03-06</h1>
                <div class="entry">
                    <ul>
	                    <li>Fixed issue that could cause Ditto to not disconnect from the clipboard</li>
                    </ul>
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
            </div>
            
            <div class="post">
                <h1 class="title">3.6.0.0 6-06-06</h1>
                <div class="entry">
                    <ul>
                    	<li>Added the ability to edit clips</li>
                    </ul>
              </div>
            </div>

            <div class="post">
                <h1 class="title">3.5.0.0 5-09-06</h1>
                <div class="entry">
                    <ul>
                    	<li>Added option to import/export clips</li>
						<li>Added menu option to clear quick paste text for selected clips</li>
                    </ul>
              </div>
            </div>
            
            <div class="post">
                <h1 class="title">3.4.0.0 4-26-06</h1>
                <div class="entry">
                    <ul>
						<li>Compact database button in options now compacts the sqlite database</li>
                        <li>Added option to prompt when deleting clips (options - Quick paste - prompt when deleting clips)</li>
                        <li>Fixed error where Ditto would show "NO TARGET" when some Outlook windows would show</li>
                        <li>Fixed shortcut key Alt-Home and removed short cut key Alt-End</li>
                        <li>Fixed crash when many clips are selected and pasted</li>
                        <li>Added Dutch translation</li>
                    </ul>
              </div>
            </div>
            
            <div class="post">
                <h1 class="title">3.3.0.0 3-26-06</h1>
                <div class="entry">
                    <ul>
                         <li>Added ability to send files from one computer to another. Just copy the file send the clip to the other computer, on paste Ditto will copy file from computer A to B and past the file</li>
                    </ul>
              </div>
            </div>
            
            <div class="post">
                <h1 class="title">3.2.0.0 2-17-06</h1>
                <div class="entry">
                    <ul>
                        <li>Fixed error on multiple pastes in unicode, the line fide was inserted properly</li>
                        <li>Fixed error that caused the "run on start" to not get entered properly in the registry</li>
                        <li>Fixed Error where if an item had a thumbnail with the clip then the clip was delete the thumbnail would still show in the list</li>
                    </ul>
              </div>
            </div>
            
            <div class="post">
                <h1 class="title">3.1.0.0 1-22-06</h1>
                <div class="entry">
                    <ul>
                        <li>added special processing for CF_HDROP (copied file from windows explorer). When copied the name of the file copied will show in ditto. (it used to only show CF_HDROP</li>
                        <li>added CF_HDROP to the default save types</li>
                        <li>fixed error when pasting and auto hide is selected ditto would not auto hide on paste</li>
                        <li>when pasting from quick paste as soon as a known quick paste is typed paste it<li>
                        <li>fixed extra character from showing when using a foreign language</li>
                    </ul>
              </div>
            </div>
            
            <div class="post">
                <h1 class="title">2.6.7.0 1-24-06</h1>
                <div class="entry">
                    <ul>
                        <li>Added number pad keys to the 0-9 shortcut keys for pasting last 10 entries</li>
                        <li>Fixed uninitialized variable that could cause Ditto to not connect to the clipboard</li>
                        <li>Fixed error that could have caused hidden icons on the desktop to show up</li>                       
                    </ul>
              </div>
            </div>
            
            <div class="post">
                <h1 class="title">2.6.5.0 11-1-05</h1>
                <div class="entry">
                    <ul>
                        <li>Only show an error message once when we can't auto send a clip to a client</li>
                        <li>Added French translation</li>
                        <li>Hide tooltip window when keys are pressed and the focus is on the search edit box</li>
                        <li>Set check marks on the menu correctly when using a foreign language</li>
                    </ul>
              </div>
            </div>
            
            <div class="post">
                <h1 class="title">2.6.4.0 10-28-05</h1>
                <div class="entry">
                    <ul>
                        <li>Added option to have ditto ensure it's always connected to the clipboard. (Options - General)</li>
                        <li>Added Italian translation</li>
                        <li>Changed how Ditto pings the clipboard to ensure it is connected to the clipboard.  It was causing a conflict with some other programs that resulted in Ditto using 100% of the cpu</li>
                        <li>Fixed disconnect from clipboard. It would not disconnect.</li>
                        <li>Fixed crash when auto updating.</li>
                        <li>Replaced search combo with search edit box.</li>
                    </ul>
              </div>
            </div>
            
            <div class="post">
                <h1 class="title">2.6.3.0 10-3-05</h1>
                <div class="entry">
                    <ul>
                        <li>Modified German Language File</li>
                        <li>Fix memory leak when showing a clip's description (F3)</li>
                    </ul>
              </div>
            </div>
            
            <div class="post">
                <h1 class="title">2.6.2.0 9-19-05</h1>
                <div class="entry">
                    <ul>
                        <li>Modified dialogs to fit foreign language text better</li>
                        <li>Added German Language File</li>
                        <li>Added more delay when clip changes happen.  I think this will help with Ditto maxing out the cpu </li>
                    </ul>
              </div>
            </div>
            
            <div class="post">
                <h1 class="title">2.6.0.0 Beta 7-24-05</h1>
                <div class="entry">
                    <ul>
                        <li>Added support for multiple languages</li>
                        <li>Added delay time for saving clips, if copies happen to fast Ditto will ignore them. Settable on General Tab in Options.</li>
                        <li>When focus is on the search combo, copy and delete will now work.</li>
                    </ul>
              </div>
            </div>
            
            <div class="post">
                <h1 class="title">2.5.1.0 - 5-12-05</h1>
                <div class="entry">
                    <ul>
                        <li>Added size limit on a clip, if a clip is over that limit the clip will not be saved</li>
                        <li>Back button will refresh the screen and go back to parent group</li>
                        <li>When focus is on the description window then you click on another window, Ditto will hide it's window correctly</li>
                    </ul>
              </div>
            </div>
            
            <div class="post">
                <h1 class="title">2.5.0.0 - 4-21-05</h1>
                <div class="entry">
                    <ul>
                        <li>Show full Rich Text or Text in description window (F3)</li>
                        <li>Added the ability to tab to description text and copy text from it</li>
                        <li>Added option to paste html format as plain text</li>
                        <li>Added option to 'Find as you Type'</li>
                        <li>Added option to always display ditto so the entire window is visible</li>
                        <li>Added option to not show clips that are in Groups in the main list</li>
                        <li>Clips are encrypted when sending across the network</li>
                        <li>Ditto must be updated on both sides for the network transfer to work</li>
                        <li>Added the ability to draw RTF text in the clip list</li>
                        <li>Added option to not send paste to active Window</li>
                        <li>Can specify a sound to play when Ditto saves a clip from the clipboard</li>
                        <li>Added shortcut key Shift - Enter to put only CF_TEXT on the clipboard and send a paste</li>
                        <li>Can use the windows keys in shortcuts</li>
                    </ul>
              </div>
            </div>
            
            <div class="post">
                <h1 class="title">2.4 - 11-30-04</h1>
                <div class="entry">
                    <ul>
                        <li>Added Help file (Right-Click -> Help)</li>
                        <li>If CF_DIB (bitmaps) is saved a thumbnail of the image will be displayed in the list and in the description (option to display image or not, Options -> Quick Paste)</li>
                        <li>Added option to not start up the server thread (Options -> Friends -> Disable Receiving Clips)</li>
                        <li>Set the font of the list control (Options -> Quick Paste, Right-Click -> Quick Options)</li>
                        <li>Properties window is resizable</li>
                        <li>When searching Spaces are treated as OR unless in quotes</li>
                        <li>Added a move to group dialog (Right-Click -> Groups -> Move To Group)</li>
                        <li>Added menu option to paste CF_TEXT(Paint Text) only (Right-Click -> Paste Plain Text Only)</li>
                        <li>Check if ditto window is in a valid monitor when showing ditto</li>
                        <li>Added CF_DIB (bitmaps) to the default types</li>
                        <li>Fixed memory leak due to improper termination of MTServerThread.</li>
                        <li>Fixed "corrupted QuickPaste position when closing while minimized".</li>
                        <li>Fixed auto internet update, it was being checked properly.</li>
                        <li>Fixed bug if you used global hot keys to paste the last item in the list it would not paste.</li>
                    </ul>
              </div>
            </div>
            
            <div class="post">
                <h1 class="title">2.3 - 7-2-04</h1>
                <div class="entry">
                    <ul>
                        <li>Keep multiple Computers' clipboards in sync (Friends Tab)</li>
                        <li>Send copied data automatically to other machines (Friends Tab)</li>
                        <li>Right click and send individual clips to other machines</li>
                        <li>Single click on the icon opens ditto</li>
                        <li>New method for getting the current active window to paste into</li>
                        <li>Option to close ditto on hot key if ditto is already active (General Tab, On by default)</li>
                        <li>Menu option to disconnect/connect ditto to the clipboard chain</li>
                        <li>If DAO is not installed you will be prompted to download the DAO redistribution package from http://ditto-cp.sourceforge.net/dao_setup.exe</li>
                    </ul>
              </div>
            </div>
            
            <div class="post">
                <h1 class="title">2.2 - 2-29-04</h1>
                <div class="entry">
                    <ul>
                        <li>Added global hot keys for the last 10 items copied</li>
                        <li>Added back button when viewing groups</li>
                        <li>Added menu options to set to</li>
                        <li>Never Auto Delete</li>
                        <li>Auto Delete</li>
                        <li>Remove Hot key</li>
                        <li>Fixed bug, when setting the group in the properties screen it was not setting the item to Never Auto Delete</li>
                        <li>Fixed bug, checking the wrong url for update</li>
                    </ul>
              </div>
            </div>
            
             <div class="post">
                <h1 class="title">2.1 - 2-15-04</h1>
                <div class="entry">
                    <ul>
                        <li>Improved drawing speed</li>
                    </ul>
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
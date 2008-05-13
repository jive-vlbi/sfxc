//Variables to set
between=28 //The pixel between the menus and the submenus
mainheight=25 //The height of the mainmenus
subheight=22 //The height of the submenus
pxspeed=13 //The pixel speed of the animation
timspeed=15 //The timer speed of the animation
menuy=0 //The top placement of the menu.
menux=0 //The left placement of the menu

//Images - Play with these

level0_regular="images/level0_regular.gif"
level0_round="images/level0_round.gif"
level0_regular="images/barmain.png"
level0_round="images/barmain.png"

level1_regular="images/level1_regular.gif"
level1_round="images/level1_round.gif"
level1_sub="images/level1_sub.gif"
level1_sub_round="images/level1_sub_round.gif"
level1_round2="images/level1_round2.gif"
level2_regular="images/level2_regular.gif"
level2_round="images/level2_round.gif"
bartop="images/bartop.png"
barbottom="images/barbottom.png"
barleft="images/barleft.png"

//Leave this line
preLoadBackgrounds(level0_regular,level0_round,level1_regular,level1_round,level1_sub,level1_sub_round,level1_round2,level2_regular,level2_round, bartop, barbottom, barleft)

//Starting the menu
onload=SlideMenuInit;

// Menu 0
makeMenu('begin')
makeMenu('top', '')
makeMenu('top', 'Home')
	makeMenu('sub', 'Welcome', 'static/welcome.html','myFrame')
	makeMenu('sub', 'Overview', 'static/overview.html','myFrame')

makeMenu('top', 'Runtime statistics')
	makeMenu('sub', 'INPUTNODE')
		makeMenu('sub2', 'Demo description', 'demos/sc07/description.html','myFrame')
		makeMenu('sub2', 'Testbed description', 'demos/sc07/testbed.html','myFrame')
		makeMenu('sub2', 'Try it yourself !', 'demos/sc07/thedemo.html','myFrame')
	makeMenu('sub', 'CORRELATION NODE')
		makeMenu('sub2', 'Demo description', 'demos/ast/description.html','myFrame')
		makeMenu('sub2', 'Testbed description', 'demos/ast/testbed.html','myFrame')
		makeMenu('sub2', 'Try it yourself !', 'demos/ast/thedemo.html','myFrame')

makeMenu('top', '')
makeMenu('end')

	


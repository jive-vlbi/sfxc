\def\currversion{April 4, 2004}                        

  %****************************************************************%
  %*                                                              *%
  %*     Prepared by Amy Hendrickson, TeXnology Inc.              *%
  %*     Copyright (C) 1996 by Amy Hendrickson, TeXnology Inc.,   *%
  %*     All rights reserved.                                     *%
  %*                                                              *%
  %*     TeXnology Inc.: 617 738-8029                             *%
  %*                     amyh@texnology.com                       *%
  %*                                                              *%   
  %*     Technical help:  dthelp@wkap.com                         *%           
  %*                                                              *%
  %****************************************************************%

  %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
  %% Macro Contents:
  %% Dimensions
  %% Global Parameters
  %% LaTeX2.09/LaTeX2e Switch, font family declarations
  %% Listing
  %% Title Pages, List of Contributors, Preface, Introduction
  %% Part, Chapter, and Appendix Commands 
  %% Counters
  %% Header Level Names
  %% Section Commands 
  %% Verse, Quote, Extract 
  %% Math
  %% Figure and Table Captions 
  %% Table of Contents, List of Figures, LOT 
  %% Bibliography, References 
  %% Footnotes
  %% Running Heads
  %% Fonts for Specific Places
  %% Algorithm
  %% Glossary
  %% Index
  %% Draft line
  %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

%%%%%%%%%%%%%%
%% To change the level of heading that will print in the Table of Contents
%% search for \setcounter{tocdepth}{0} below, and change it if you have
%% a strong preference to have more levels of headings appear in the toc.
%% Default is \setcounter{tocdepth}{1} and that is the level Kluwer prefers.

%% \setcounter{tocdepth}{0} means Chap. headings        will print in TOC
%% \setcounter{tocdepth}{1} means Chap. and section headings         "
%% \setcounter{tocdepth}{2} means Chap. sect, and subsect headings   "
%% \setcounter{tocdepth}{3} means Chap. sect, subsect and subsubsect "

\typeout{^^J^^J
Kluwer Academic Press^^J
Edited Book Style^^J
Written by Amy Hendrickson, TeXnology, Inc., 3/96^^J
This version is dated \currversion^^J^^J}

%% Hack to make SWP handle cross referencing in equations
\let\saveequation\equation
\let\saveeqnarray\eqnarray

%% Dimensions for both 2e and 2.09 ===>>>

\newif\ifll
\expandafter\ifx\csname LaTeXe\endcsname\relax % LaTeX2.09 is used
\else% LaTeX2e is being used, so set ll true
\global\lltrue\fi

\ifll
% To make these commands work with earlier versions of 2e.
\gdef\hb@xt@{\hbox to}
%
\expandafter\ifx\csname @star@or@long\endcsname\relax
\gdef\DeclareRobustCommand{\@star@or@long\declare@robustcommand}
\gdef\declare@robustcommand#1{%
   \ifx#1\@undefined\else\ifx#1\relax\else
      \@latex@info{Redefining \string#1}%
   \fi\fi
   \edef\reserved@a{\string#1}%
   \def\reserved@b{#1}%
   \edef\reserved@b{\expandafter\strip@prefix\meaning\reserved@b}%
   \edef#1{%
      \ifx\reserved@a\reserved@b
         \noexpand\x@protect
         \noexpand#1%
      \fi
      \noexpand\protect
      \expandafter\noexpand\csname
         \expandafter\@gobble\string#1 \endcsname
   }%
   \let\@ifdefinable\@rc@ifdefinable
   \expandafter\new@command\csname
      \expandafter\@gobble\string#1 \endcsname
}
\gdef\@star@or@long#1{%
  \@ifstar
   {\let\l@ngrel@x\relax#1}%
   {\let\l@ngrel@x\long#1}}
%
\def\@star@or@long#1{%
  \@ifstar
   {\let\l@ngrel@x\relax#1}%
   {\let\l@ngrel@x\long#1}}
%% added Dec 9==>>
\let\l@ngrel@x\relax
\def\newcommand{\@star@or@long\new@command}
\def\new@command#1{%
  \@testopt{\@newcommand#1}0}
\def\@newcommand#1[#2]{%
  \@ifnextchar [{\@xargdef#1[#2]}%
                {\@argdef#1[#2]}}
\long\def\@argdef#1[#2]#3{%
   \@ifdefinable #1{\@yargdef#1\@ne{#2}{#3}}}
\long\def\@xargdef#1[#2][#3]#4{%
  \@ifdefinable#1{%
     \expandafter\def\expandafter#1\expandafter{%
          \expandafter
          \@protected@testopt
          \expandafter
          #1%
          \csname\string#1\expandafter\endcsname
          {#3}}%
       \expandafter\@yargdef
          \csname\string#1\endcsname
           \tw@
           {#2}%
           {#4}}}
\def\@testopt#1#2{%
  \@ifnextchar[{#1}{#1[#2]}}
\def\@protected@testopt#1{%%
  \ifx\protect\@typeset@protect
    \expandafter\@testopt
  \else
    \@x@protect#1%
  \fi}
\long\def\@yargdef#1#2#3{%
  \@tempcnta#3\relax
  \advance \@tempcnta \@ne
  \let\@hash@\relax
  \edef\reserved@a{\ifx#2\tw@ [\@hash@1]\fi}%
  \@tempcntb #2%
  \@whilenum\@tempcntb <\@tempcnta
     \do{%
         \edef\reserved@a{\reserved@a\@hash@\the\@tempcntb}%
         \advance\@tempcntb \@ne}%
  \let\@hash@##%
  \l@ngrel@x\expandafter\def\expandafter#1\reserved@a}
\long\def\@reargdef#1[#2]#3{%
  \@yargdef#1\@ne{#2}{#3}}
\def\renewcommand{\@star@or@long\renew@command}
\def\renew@command#1{%
  {\escapechar\m@ne\xdef\@gtempa{{\string#1}}}%
  \expandafter\@ifundefined\@gtempa
     {\@latex@error{\string#1 undefined}\@ehc}%
     {}%
  \let\@ifdefinable\@rc@ifdefinable
  \new@command#1}
\long\def\@ifdefinable #1#2{%
      \edef\reserved@a{\expandafter\@gobble\string #1}%
     \@ifundefined\reserved@a
         {\edef\reserved@b{\expandafter\@carcube \reserved@a xxx\@nil}%
          \ifx \reserved@b\@qend \@notdefinable\else
            \ifx \reserved@a\@qrelax \@notdefinable\else
              #2%
            \fi
          \fi}%
         \@notdefinable}
\let\@@ifdefinable\@ifdefinable
\long\def\@rc@ifdefinable#1#2{%
  \let\@ifdefinable\@@ifdefinable
  #2}
\fi %<=== end of conditional to see if above code needs to be
%                      added for users of older versions of 2e
%%
\else
\def\nobreakspace{~}
\gdef\@plus{plus}
\gdef\@minus{minus}
\fi

\let\savefootnote\footnote
\let\savefootnotetext\footnotetext

\parindent=1pc
\parskip=0pt 

\textheight = 45.5pc% Suzanne asked for 45.5, was 41pc

\textwidth 28.5pc %27pc in specs, 28.5 in old version
\newdimen\savetextwidth
\savetextwidth=\textwidth

\clubpenalty10000
\widowpenalty10000

\topmargin.75in
\headheight 12pt
\headsep 16pt
\topskip  0pt
\footskip 16pt

\columnsep 10pt
\columnseprule 0pt

\footnotesep 6.65pt
\skip\footins 24pt plus 4pt minus 2pt


\floatsep 12pt plus 2pt minus 2pt
\textfloatsep 20pt plus 2pt minus 4pt
\intextsep 12pt  plus 2pt minus 2pt

\dblfloatsep 12pt plus 2pt minus 2pt
\dbltextfloatsep 20pt plus 2pt minus 4pt

\ifll\else
\global\@maxsep 20pt
\global\@dblmaxsep 20pt
\fi

\@fptop 0pt plus 1fil
\@fpsep 8pt plus 2fil
\@fpbot 0pt plus 1fil
\@dblfptop 0pt plus 1fil
\@dblfpsep 8pt plus 2fil
\@dblfpbot 0pt plus 1fil

\maxdepth=4pt %% 2e default is .5\topskip

\oddsidemargin 2.26cm % 2pc=no extra space on left
\evensidemargin 2.26cm %  2pc=no extra space on left
\marginparwidth .75in


\if@twocolumn
 \setlength\marginparsep {10\p@}
\else
  \setlength\marginparsep{7\p@}
\fi
\setlength\marginparpush{5\p@}

\setlength\partopsep{2\p@ \@plus 1\p@ \@minus 1\p@}

%%%
\setcounter{topnumber}{3}
\def\topfraction{.99}
\setcounter{bottomnumber}{3}
\def\bottomfraction{.9}
\setcounter{totalnumber}{9}
\def\textfraction{.01}
\def\floatpagefraction{.5}
\setcounter{dbltopnumber}{2}
\def\dbltopfraction{.7}
\def\dblfloatpagefraction{.5}
%%%

\setlength\arraycolsep{5\p@}
\tabcolsep 6pt


\setlength\arrayrulewidth{.4\p@}
\setlength\doublerulesep{2\p@}
\setlength\tabbingsep{\labelsep}
\skip\@mpfootins = \skip\footins
\setlength\fboxsep{3\p@}
\setlength\fboxrule{.4\p@}

\setlength\columnsep{10\p@}
\setlength\columnseprule{0\p@}

%%%%%%%%%%%%%%% <<== end dimensions

%%% Global parameters ==>>

%% to make left and right page position differently.
\@twosidetrue  \@mparswitchtrue
\@twocolumnfalse

%% 2e ==>>>
\newif\if@openbib
%\@openbibtrue
\@openbibfalse
\newif\if@openright
\newif\if@mainmatter 
\@mainmattertrue
\newif\if@restonecol
\newif\if@titlepage

%% Start new chapter on right side
\newif\if@openright
\@openrighttrue
%% <<== end 2e

\newcommand{\contentsname}{Contents}
\newcommand{\listfigurename}{List of Figures}
\newcommand{\listtablename}{List of Tables}
\newcommand{\bibname}{Bibliography}
\newcommand{\indexname}{Index}
\newcommand{\figurename}{Fig.}
\newcommand{\tablename}{Table}
\newcommand{\partname}{Part}
\newcommand{\chaptername}{Chapter}
\newcommand{\appendixname}{Appendix}

%%% <== end global parameters


%%% LaTeX2.09/LaTeX2e Switch  ====>>>

\font\egtbf=cmbx8
\font\sevbf=cmbx7
\font\fivbf=cmbx5

\ifll \let\dooptions\ProcessOptions

\DeclareOption{a4paper}{%
  \AtEndOfPackage{\global\advance\textheight by 48pt}}
\else
\let\dooptions\@options\fi
\dooptions

\def\@xiiipt{13}

\ifll
\NeedsTeXFormat{LaTeX2e}
\ProvidesClass{kapedbk}[1999/07/14]
\newcommand{\@ptsize}{}
\renewcommand{\normalsize}{%
   \@setfontsize\normalsize\@xipt\@xiiipt
   \abovedisplayskip 10\p@ \@plus2\p@ \@minus5\p@
   \abovedisplayshortskip \z@ \@plus3\p@
   \belowdisplayshortskip 6\p@ \@plus3\p@ \@minus3\p@
   \belowdisplayskip \abovedisplayskip
   \let\@listi\@listI}
\normalsize
\newcommand{\small}{%
   \@setfontsize\small\@ixpt{11}%
   \abovedisplayskip 8.5\p@ \@plus3\p@ \@minus4\p@
   \abovedisplayshortskip \z@ \@plus2\p@
   \belowdisplayshortskip 4\p@ \@plus2\p@ \@minus2\p@
   \def\@listi{\leftmargin\leftmargini
               \topsep 4\p@ \@plus2\p@ \@minus2\p@
               \parsep 2\p@ \@plus\p@ \@minus\p@
               \itemsep \parsep}%
   \belowdisplayskip \abovedisplayskip
}
\newcommand{\footnotesize}{%
   \@setfontsize\footnotesize\@viiipt{9.5}%
   \abovedisplayskip 6\p@ \@plus2\p@ \@minus4\p@
   \abovedisplayshortskip \z@ \@plus\p@
   \belowdisplayshortskip 3\p@ \@plus\p@ \@minus2\p@
   \def\@listi{\leftmargin\leftmargini
               \topsep 3\p@ \@plus\p@ \@minus\p@
               \parsep 2\p@ \@plus\p@ \@minus\p@
               \itemsep \parsep}%
   \belowdisplayskip \abovedisplayskip
}
\newcommand{\scriptsize}{\@setfontsize\scriptsize\@viipt\@viiipt}
\newcommand{\tiny}{\@setfontsize\tiny\@vpt\@vipt}
\newcommand{\large}{\@setfontsize\large\@xiipt{14}}
\newcommand{\Large}{\@setfontsize\Large\@xivpt{18}}
\newcommand{\LARGE}{\@setfontsize\LARGE\@xviipt{22}}
\newcommand{\huge}{\@setfontsize\huge\@xxpt{25}}
\newcommand{\Huge}{\@setfontsize\Huge\@xxvpt{30}}

\@maxdepth\maxdepth
\DeclareOldFontCommand{\rm}{\normalfont\rmfamily}{\mathrm}
\DeclareOldFontCommand{\sf}{\normalfont\sffamily}{\mathsf}
\DeclareOldFontCommand{\tt}{\normalfont\ttfamily}{\mathtt}
\DeclareOldFontCommand{\bf}{\normalfont\bfseries}{\mathbf}
\DeclareOldFontCommand{\it}{\normalfont\itshape}{\mathit}
\DeclareOldFontCommand{\sl}{\normalfont\slshape}{\@nomath\sl}
\DeclareOldFontCommand{\sc}{\normalfont\scshape}{\@nomath\sc}
\DeclareRobustCommand{\cal}{\@fontswitch{\relax}{\mathcal}}
\DeclareRobustCommand{\mit}{\@fontswitch{\relax}{\mathnormal}}

\else %% LaTeX2.09
%% To get bold super and sub-scripts in the right size:
\gdef\xpt{\textfont\z@\tenrm
  \scriptfont\z@\sevrm \scriptscriptfont\z@\fivrm
\textfont\@ne\tenmi \scriptfont\@ne\sevmi \scriptscriptfont\@ne\fivmi
\textfont\tw@\tensy \scriptfont\tw@\sevsy \scriptscriptfont\tw@\fivsy
\textfont\thr@@\tenex \scriptfont\thr@@\tenex \scriptscriptfont\thr@@\tenex
\def\unboldmath{\everymath{}\everydisplay{}\@nomath\unboldmath
          \textfont\@ne\tenmi
          \textfont\tw@\tensy \textfont\lyfam\tenly
          \@boldfalse}\@boldfalse
\def\boldmath{\@ifundefined{tenmib}{\global\font\tenmib\@mbi
   \global\font\tensyb\@mbsy
   \global\font\tenlyb\@lasyb\relax\@addfontinfo\@xpt
   {\def\boldmath{\everymath{\mit}\everydisplay{\mit}\@prtct\@nomathbold
        \textfont\@ne\tenmib \textfont\tw@\tensyb
        \textfont\lyfam\tenlyb \@prtct\@boldtrue}}}{}\@xpt\boldmath}%
\def\prm{\fam\z@\tenrm}%
\def\pit{\fam\itfam\tenit}\textfont\itfam\tenit \scriptfont\itfam\sevit
    \scriptscriptfont\itfam\sevit
\def\psl{\fam\slfam\tensl}\textfont\slfam\tensl
     \scriptfont\slfam\tensl \scriptscriptfont\slfam\tensl
\def\pbf{\fam\bffam\tenbf}\textfont\bffam\tenbf
    \scriptfont\bffam\sevbf \scriptscriptfont\bffam\fivbf
\def\ptt{\fam\ttfam\tentt}\textfont\ttfam\tentt
    \scriptfont\ttfam\tentt \scriptscriptfont\ttfam\tentt
\def\psf{\fam\sffam\tensf}\textfont\sffam\tensf
    \scriptfont\sffam\tensf \scriptscriptfont\sffam\tensf
\def\psc{\@getfont\psc\scfam\@xpt{\@mcsc}}%
\def\ly{\fam\lyfam\tenly}\textfont\lyfam\tenly
   \scriptfont\lyfam\sevly \scriptscriptfont\lyfam\fivly
\@setstrut \rm}
%
\gdef\@normalsize{\@setsize\normalsize{13pt}\xipt\@xipt
\abovedisplayskip 10\p@ plus2\p@ minus5\p@
\belowdisplayskip \abovedisplayskip
\abovedisplayshortskip  \z@ plus3\p@
\belowdisplayshortskip  6\p@ plus3\p@ minus3\p@
\let\@listi\@listI}

\gdef\small{\@setsize\small{11pt}\ixpt\@ixpt
\abovedisplayskip 8.5\p@ plus3\p@ minus4\p@
\belowdisplayskip \abovedisplayskip
\abovedisplayshortskip \z@ plus2\p@
\belowdisplayshortskip 4\p@ plus2\p@ minus2\p@
\def\@listi{\leftmargin\leftmargini
\topsep 4\p@ plus2\p@ minus2\p@\parsep 2\p@ plus\p@ minus\p@
\itemsep \parsep}}

\gdef\footnotesize{\@setsize\footnotesize{9.5pt}\viiipt\@viiipt
\abovedisplayskip 6\p@ plus2\p@ minus4\p@
\belowdisplayskip \abovedisplayskip
\abovedisplayshortskip \z@ plus\p@
\belowdisplayshortskip 3\p@ plus\p@ minus2\p@
\def\@listi{\leftmargin\leftmargini
\topsep 3\p@ plus\p@ minus\p@\parsep 2\p@ plus\p@ minus\p@
\itemsep \parsep}}
%
\gdef\scriptsize{\@setsize\scriptsize{8pt}\viipt\@viipt}
\gdef\tiny{\@setsize\tiny{6pt}\vpt\@vpt}
\gdef\large{\@setsize\large{14pt}\xiipt\@xiipt}
\gdef\Large{\@setsize\Large{18pt}\xivpt\@xivpt}
\gdef\LARGE{\@setsize\LARGE{22pt}\xviipt\@xviipt}
\gdef\huge{\@setsize\huge{25pt}\xxpt\@xxpt}
\gdef\Huge{\@setsize\Huge{30pt}\xxvpt\@xxvpt}
%
\gdef\bfseries{\bf}
\fi

\lineskip 1pt
\normallineskip 1pt
\def\baselinestretch{1}

\normalsize
%% <<== end font info


%% Listing, same as 2.09 ==>>

\def\@listI{\leftmargin\leftmargini
            \parsep 4\p@ \@plus2\p@ \@minus\p@
            \topsep 8\p@ \@plus2\p@ \@minus4\p@
            \itemsep4\p@ \@plus2\p@ \@minus\p@}
\let\@listi\@listI
\@listi
\def\@listii {\leftmargin\leftmarginii
              \labelwidth\leftmarginii
              \advance\labelwidth-\labelsep
              \topsep    4\p@ \@plus2\p@ \@minus\p@
              \parsep    2\p@ \@plus\p@  \@minus\p@
              \itemsep   \parsep}
\def\@listiii{\leftmargin\leftmarginiii
              \labelwidth\leftmarginiii
              \advance\labelwidth-\labelsep
              \topsep    2\p@ \@plus\p@\@minus\p@
              \parsep    \z@
              \partopsep \p@ \@plus\z@ \@minus\p@
              \itemsep   \topsep}
\def\@listiv {\leftmargin\leftmarginiv
              \labelwidth\leftmarginiv
              \advance\labelwidth-\labelsep}
\def\@listv  {\leftmargin\leftmarginv
              \labelwidth\leftmarginv
              \advance\labelwidth-\labelsep}
\def\@listvi {\leftmargin\leftmarginvi
              \labelwidth\leftmarginvi
              \advance\labelwidth-\labelsep}

\leftmargini 25pt
\leftmarginii 22pt
\leftmarginiii 18.7pt
\leftmarginiv 17pt
\leftmarginv 10pt
\leftmarginvi 10pt

\leftmargin\leftmargini
\labelsep 5pt
\labelwidth\leftmargini\advance\labelwidth-\labelsep

\if@twocolumn
  \setlength\leftmargini  {2em}
\else
  \setlength\leftmargini  {2.5em}
\fi
\setlength\leftmarginii  {2.2em}
\setlength\leftmarginiii {1.87em}
\setlength\leftmarginiv  {1.7em}
\if@twocolumn
  \setlength\leftmarginv  {.5em}
  \setlength\leftmarginvi {.5em}
\else
  \setlength\leftmarginv  {1em}
  \setlength\leftmarginvi {1em}
\fi
\setlength\leftmargin    {\leftmargini}
\setlength  \labelsep  {.5em}
\setlength  \labelwidth{\leftmargini}
\addtolength\labelwidth{-\labelsep}

\@beginparpenalty -\@lowpenalty
\@endparpenalty   -\@lowpenalty
\@itempenalty     -\@lowpenalty

\renewcommand{\theenumi}{\arabic{enumi}}
\renewcommand{\theenumii}{\alph{enumii}}
\renewcommand{\theenumiii}{\roman{enumiii}}
\renewcommand{\theenumiv}{\Alph{enumiv}}
\newcommand{\labelenumi}{\theenumi}
\newcommand{\labelenumii}{(\theenumii)}
\newcommand{\labelenumiii}{\theenumiii}
\newcommand{\labelenumiv}{\theenumiv}
\renewcommand{\p@enumii}{\theenumi}
\renewcommand{\p@enumiii}{\theenumi(\theenumii)}
\renewcommand{\p@enumiv}{\p@enumiii\theenumiii}

\def\sqbullet{\raise.2ex\hbox{\vrule width 4pt height4pt}}

\def\labelitemi{\llap{\hbox to8pt{\sqbullet\hfill}}}
%\newcommand{\labelitemi}{$\m@th\bullet$}
\newcommand{\labelitemii}{\normalfont\bfseries --}
\newcommand{\labelitemiii}{$\m@th\ast$}
\newcommand{\labelitemiv}{$\m@th\cdot$}
\newenvironment{description}
               {\list{}{\labelwidth\z@ \itemindent-\leftmargin
                        \let\makelabel\descriptionlabel}}
               {\endlist}

\ifll
\newcommand*{\descriptionlabel}[1]{\hspace\labelsep
                                \normalfont\bfseries #1}
\else
\gdef\descriptionlabel#1{\hspace\labelsep \bf #1}\fi

%%% <<=== end of listing commands 

%% Title Pages, List of Contributors, Preface, Introduction==>>


\def\name#1{\vskip2pt\fakesc #1*\hskip10pt}

\newif\ifpreface
\def\preface{\chapter*{Preface}
\global\prefacetrue
\addcontentsline{toc}{spchapter}{\protect\cnumberline{}Preface}
\global\titletrue\rhead{PREFACE}}

\def\endpreface{\newpage\rhead{}}


\def\prefaceauthor#1{\vskip14pt\hbox to\textwidth{\hfill
\vtop{\let\\ \cr
\halign{##\hfill\cr
\sc #1
\crcr}
}}}

\def\foreword{\chapter*{Foreword}
\addcontentsline{toc}{spchapter}{\protect\cnumberline{}Foreword}
\global\titletrue\rhead{FOREWORD}}

\let\endforeword\endpreface

\def\forewordauthor#1{\vskip14pt\hbox to\textwidth{\hfill
\vtop{\let\\ \cr
\halign{##\hfill\cr
#1
\crcr}
}}}

\let\forwardauthor\forewordauthor

\newif\ifdedi
\def\dedication#1{\newpage
\global\deditrue
\global\titletrue\null\vskip15pc
\noindent\hfill\vtop{\hsize=10pc
\leftskip=\parfillskip
\baselineskip=12pt
\parfillskip=0pt
\dedicationfont
\noindent#1\vskip1sp}
\addcontentsline{toc}{spchapter}{\protect\cnumberline{}Dedication}
\thispagestyle{empty}
\setcounter{page}{5}
\newpage
\rhead{}}


\def\contributingauthors{\chapter*{Contributing Authors}
\addcontentsline{toc}{spchapter}{\protect\cnumberline{}Contributing Authors}
\global\titletrue
\rightrunninghead{Contributing Authors}
\vskip1sp\vskip-16pt
\def\author##1{\vskip18pt
\noindent{\bf\relax ##1}}}

\def\endcontributingauthors{\newpage
\markboth{}{}}

\def\introduction#1{\newpage
\bgroup
\parfillskip=0pt
\chapter*{\chaptertitlefont Introduction}
\vskip1sp
\vskip-11pc
\noindent{\authorfont #1}\egroup
\vskip12.5pc
\rightrunninghead{INTRODUCTION}
\renewcommand{\thechapter}{I}
\addcontentsline{toc}{chapter}{\protect\cnumberline{}Introduction}
{\let\\ \space \let\footnote\eatone\gdef\totoc{\vskip2pt\noindent\tocauthorfont #1}}
\addtocontents{toc}{{\string\noindent\totoc}}
\indent\ignorespaces}

%% from 2e
\newcommand{\frontmatter}{\cleardoublepage
            \@mainmatterfalse\pagenumbering{roman}}
\newcommand{\mainmatter}{\cleardoublepage
       \@mainmattertrue\pagenumbering{arabic}}
\newcommand{\backmatter}{\if@openright\cleardoublepage\else\clearpage\fi
      \@mainmatterfalse}
%%

\newif\iftitle
\def\booktitle{\@ifnextchar[\@booktitle\sbooktitle}

\def\@booktitle[#1]#2{%
{\let\\\par
\gdef\thetitle{\uppercase{#2}}}
\gdef\theshorttitle{\uppercase{#1}}
\gdef\lrunninghead{\theshorttitle}}

\def\sbooktitle#1{%
{\let\\ =\par
\gdef\thetitle{\uppercase{#1}}}
{\let\\ =\ \gdef\lrunninghead{\uppercase{#1}}}
}

\def\subtitle#1{\def\thesubtitle{#1}}

\newcount\edcount
\newcount\loopnum

\def\editor#1{\global\advance\edcount by1\relax
\expandafter\def\csname theeditor\the\edcount\endcsname{\uppercase{#1}}}

\def\edaffil#1{\expandafter\def\csname theedaffil\the\edcount\endcsname{#1}}

\def\booktitlerunninghead#1{\gdef\lrunninghead{#1}}
\booktitlerunninghead{}
\def\chaptitlerunninghead#1{\gdef\rrunninghead{#1}}
\chaptitlerunninghead{}
\let\rightrunninghead\chaptitlerunninghead
\let\rhead\chaptitlerunninghead
\let\lhead\booktitlerunninghead
\rhead{}

\def\markboth#1#2{\rhead{#2}}
\def\markright#1{\rhead{#1}}

\booktitle{}
\subtitle{}

\newbox\testtitleheight

\def\halftitlepage{\thispagestyle{empty}
\moveright-3pt\vbox{\vskip-30pt
\parindent=0pt
\hsize=27pc
\noindent\hbox to0pt{\vrule height 3pt width 33pc\hss}
\vskip-1pt
\baselineskip19pt
\LARGE \boldmath
\halftitlefont
\thetitle\vskip1sp}
\newpage
\thispagestyle{empty}
}

\def\seriestitle#1{\def\theseriestitle{\uppercase{#1}}}

\def\editortable#1\endeditortable{\def\theeditortable{{\professorfont
\baselineskip=10pt
\let\\=\cr
\halign{#### \hfill&#### \hfill\cr
\hskip.5\textwidth&\hskip.5\textwidth\cr
#1
\crcr}}}}

\def\seriespage{\thispagestyle{empty}
\dimen0=\vsize
\advance\dimen0-1pt
\vbox to\dimen0{\parindent=0pt
\vbox to0pt{\vss
\seriestitlefont\theseriestitle
\vskip10.5pt
{\professorfont
Editors:}}
\professorfont
\vskip4pt
\vbox{\theeditortable}
\vskip1sp
\vfill
{\professorfont
Other Books in the series:}
\seriesbookfont
\vskip4pt
\theseriesbooks
}\newpage}

\def\seriesbook#1#2{\vskip.5pt{#1:
\vskip1sp
\noindent\hskip8pt\uppercase{#2}}\vskip1sp}

\def\seriesbooks#1\endseriesbooks{%
\def\theseriesbooks{\let\book\seriesbook
#1}}


\def\titlepage{\ifodd\c@page\else
\vfill\eject\null
\thispagestyle{empty}
\newpage\fi
\thispagestyle{empty}
\dimen0=\vsize
\advance\dimen0 by-6pt
\vbox to\dimen0{\vskip-30pt
\parskip=0pt
\parindent=0pt
\hsize=27pc
\noindent\hbox to0pt{\vrule height 0pt width 33pc\hss}
\vskip-1pt
{\baselineskip19pt
\LARGE \boldmath
\booktitlefont
\thetitle\vskip1sp}
\vbox to 13pc{
\ifx\subtitle\empty\vfill\else
\vskip6pt %18pt
{\baselineskip14pt
\LARGE \boldmath
\booktitlefont \thesubtitle\vskip1sp}\vfill\fi}
\baselineskip=9pt
{\editoraffilfont
Edited by}
\vskip3pt
\loopnum=1
{\loop\ifnum\loopnum<\edcount
\ifnum\loopnum>1 \vskip12pt\fi
\editorfont \csname theeditor\the\loopnum\endcsname
\vskip1pt\let\\ =\newline
\editoraffilfont
\csname theedaffil\the\loopnum\endcsname
\global\advance\loopnum by 1
\repeat
\ifnum\loopnum>1 \vskip12pt\fi
\editorfont \csname theeditor\the\loopnum\endcsname
\vskip1pt\let\\ =\newline
\editoraffilfont
\csname theedaffil\the\loopnum\endcsname
}
\vfill
\baselineskip=10pt
\kluwerfont\obeylines
Kluwer Academic Publishers
\vskip1sp
{\locationfont
Boston/Dordrecht/London}
}
\newpage
}

\def\dotspace{\hskip5pt\raise.5ex\hbox{$\scriptscriptstyle\bullet$}\hskip5pt}

%% end Title Pages, Preface

%% Part, Chapter, and Appendix Commands ===>>>
\def\keywords{\vskip2pt{\small
\leftskip=55pt
\noindent\llap\bgroup\hbox to55pt{\bf Keywords:\hfill}}\ignorespaces}

\def\endkeywords{\vskip1sp
\leftskip=0pt\egroup
\vskip24pt}


\def\abstract{\global\c@footnote=0
\vskip24pt\bgroup\small
\leftskip=55pt
\everypar={}
\noindent\llap{\hbox to55pt{\bf Abstract\hfill}}\ignorespaces}

\def\endabstract{\vrule width0pt depth 13pt\vskip1sp\egroup
\vskip1sp}

\def\chaptermark#1{}

\def\openright{\clearpage\ifodd\c@page\else
\null\thispagestyle{empty}\newpage\fi\global\titletrue%
}


\newcommand{\part}{\notes\clearpage\ifodd\c@page\else
\null\thispagestyle{empty}\newpage\fi
                 \thispagestyle{empty}%
                     \@tempswafalse
                 \secdef\@part\@spart}

\newif\ifpartwithtext
\long\def\partwithtext{\futurelet\next\lookforbrack}
\def\lookforbrack{\ifx\next[\let\go\xpartwithtext\else
\let\go\ypartwithtext\fi\go}

\long\def\xpartwithtext[#1]#2#3{\global\partwithtexttrue\part[#1]{#2}
\vskip36pt
\noindent
{#3}\@endpart}

\long\def\ypartwithtext#1#2{\global\partwithtexttrue\part{#1}
\vskip36pt
\noindent
{#2}\@endpart}


\def\@part[#1]#2{%
\openright
\ifnum\c@chapter=0
\ifx\thepage\xpage\else
\pagenumbering{arabic}\fi\fi
      \refstepcounter{part}%
{\def\\ {\space} 
      \addcontentsline{toc}{part}{\thepart\string\hskip1em\relax#1}}%
    \markboth{}{}
    {\interlinepenalty \@M
\parindent=0pt
\baselineskip=20pt
\def\\ {\vskip1sp} 
\reset@font\partnumberfont \thepart\vskip18pt
\Large\boldmath\partfont
\edef\argtwo{\uppercase{#2}}
\argtwo\par\unboldmath}%
\ifpartwithtext\global\partwithtextfalse\else
    \@endpart\fi}

\def\@spart#1{%
    {\centering
     \interlinepenalty \@M
     \reset@font
      \bfseries #1\par}%
    \@endpart}

\def\@endpart{\vfill\newpage
              \if@twoside
                \hbox{}%
                \thispagestyle{empty}%
                \newpage
             \fi}


\def\eatone#1{}

\newif\ifandon
\def\and{\global\andontrue}

\newif\ifauthoron
\def\author{\futurelet\next\lookforbrac}

\def\lookforbrac{\ifx\next[\let\go\xauthor
\else\let\go\yauthor\fi\go}

\def\xauthor[#1]#2{\vskip24pt
{\vskip-\parskip
\parindent=0pt
\vskip3pt
\def\thefootnote{\fnsymbol{footnote}}
\let\thanks\savefootnote
\let\footnote\savefootnote
\let\\ \chapskip{\authorfont\relax #2}\vskip1sp}
{\let\\ \space \let\footnote\eatone \let\thanks\eatone 
\let\altaffilmark\eatone
\xdef\totoc{%
\ifauthoron\ifandon\string\tocauthorfont\space and\else\string\hskip-3pt,\fi%
\string\ \else\string\noindent\fi\tocauthorfont\relax #1}}
\addtocontents{toc}{{\totoc}}
\global\firstprologuetrue
\global\authorontrue
\global\andonfalse
}

\def\yauthor#1{\vskip24pt
{\vskip-\parskip
\parindent=0pt
\def\thefootnote{\fnsymbol{footnote}}
\let\thanks\savefootnote
\let\footnote\savefootnote
\let\\ \chapskip{\authorfont\relax #1}\vskip1sp}
{\let\\ \space \let\footnote\eatone \let\thanks\eatone
\let\altaffilmark\eatone
\xdef\totoc{%
\ifauthoron\ifandon\string\tocauthorfont\space and\else\string\hskip-3pt,\fi%
\string\ \else\string\noindent\fi%
\tocauthorfont\relax #1}}
\addtocontents{toc}{{\totoc}}
\global\firstprologuetrue
\global\authorontrue
\global\andonfalse
}

\def\altaffilmark#1{$^{#1}$}
\def\altaffiltext#1#2{\vskip12pt\affil{$^{#1}$#2}}

\def\affil#1{\vskip2pt{\let\email\affilemail
\parindent=0pt
\def\thefootnote{\fnsymbol{footnote}}
\let\footnote\savefootnote
\let\thanks\savefootnote
\let\\ \chapskip{\baselineskip=11pt\affilfont #1\vskip1sp}}
}

\def\ttcurlyleft{{\tt\char123}}
\def\ttcurlyright{{\tt\char125}}

\def\email#1{{\vskip1sp
\parindent=0pt
\def\thefootnote{\fnsymbol{footnote}}
\let\footnote\savefootnote
\let\thanks\savefootnote
{\let\\ \chapskip
\let\{=\ttcurlyleft \let\}=\ttcurlyright
\emailfont #1}\vskip1sp}
}

\newif\ifmultipleauthors
\def\affilemail#1{
{\global\multipleauthorstrue
\def\thefootnote{\fnsymbol{footnote}}
\let\footnote\savefootnote
\let\thanks\savefootnote
\vskip1pt
\baselineskip=13pt
\let\\ \chapskip{\emailfont #1}\vskip1sp}
\vskip8pt
}

\newif\iffirstprologue
\def\prologue#1#2{\iffirstprologue
\global\firstprologuefalse\vskip48pt\else
\vskip12pt\fi
{\leftskip=1pc \rightskip=1pc
\parindent=0pt \small\it
\let\\ \chapskip#1\vskip1sp
\hbox to\textwidth{\small\rm\hfill---#2\hskip1pc}
}\vskip4.5pc}

\@afterindenttrue

\def\chapter{\global\authoronfalse\openright
\global\c@theorem=0
\global\c@calphanum=0
\global\c@alphanum=0
\global\c@footnote=0
\global\footnum=0
\renewcommand{\thechapter}{\arabic{chapter}}
                    \global\@topnum\z@
                    \@afterindenttrue
                    \secdef\@chapter\@schapter}

\def\cnumberline#1{{\hbox{\tocarticletitlefont #1}
\vskip-\parskip\noindent%
\hskip-\leftskip\ignorespaces}}


\def\chapnumberline#1#2{\vskip4pt\noindent\hskip-\leftskip%
\vtop{\vbox to0pt{\vss\hbox{\tocarticletitlefont #1}
\vskip\baselineskip\vskip2pt}
\vskip-.9\baselineskip
\advance\hsize-36pt
\parindent=-\leftskip\parfillskip=0pt
plus1fil\relax#2\vrule depth4pt width0pt\vskip1sp}\hfill}

\def\chapskip{\vskip-\parskip}

\def\xpage{\csname @arabic\endcsname \c@page}

\newcount\ftnum
\newcount\saveftnum
\def\catchfootnote#1{%
\global\advance\ftnum by1
\expandafter\gdef\csname thecatchfootnote\the\ftnum\endcsname{#1}}

\def\@chapter[#1]#2{\global\appendonfalse
\global\ftnum=0
\setbox0=\hbox{\let\thanks\catchfootnote
\let\\ \relax\let\footnote\catchfootnote#2}
{\def\thefootnote{\fnsymbol{footnote}}
\ifnum\ftnum<1\else
\saveftnum\ftnum
\global\advance\saveftnum by1
\ftnum=1
\loop\ifnum\saveftnum>\ftnum
\advance\c@footnote by 1
\savefootnotetext{\csname thecatchfootnote\the\ftnum\endcsname}
\global\advance\ftnum by1
\repeat
\fi}
\ifnum\c@chapter=0
\ifx\thepage\xpage\else
\pagenumbering{arabic}\fi\fi
{\let\thanks\eatone \let\footnote\eatone
\ifnum \c@secnumdepth >\m@ne
                       \if@mainmatter
                         \refstepcounter{chapter}%
                         \typeout{\@chapapp\space\thechapter.}%
                         \addcontentsline{toc}{chapter}%
                                   {\protect\chapnumberline{\thechapter}{#1}}%
                       \else
                         \addcontentsline{toc}{chapter}{#1}%
                       \fi
                    \else
                      \addcontentsline{toc}{chapter}{#1}%
                    \fi
%
\setbox0=\vbox{\let\@currentlabel\thechapter
\let\footnote\eatone\let\thanks\eatone
\def\label##1{\gdef\dolabelnow{\savelabel{##1}}}
#2}%
\ifsprunningheads %
              \gdef\lrunninghead{#1}%
\else
               \rightrunninghead{#1}%
\fi
      \@makechapterhead{{\let\label\eatone
\let\\ \chapskip\def\two{\uppercase{\vspace{0.29cm}\mbox{}\\ #2}}\two}}%
}% end \let\thanks\eatone
\let\@currentlabel\thechapter
\dolabelnow
\let\dolabelnow\relax
\let\label\savelabel
            \normalsize %% to compensate for authors that don't end
           % \begin{acknowledgments}...\end, and use \acknowledgements instead
\global\c@footnote=0
\@afterheading}

\let\dolabelnow\relax

\def\dofootnotenumber#1{\global\advance\c@footnote by 1
\raise.9ex\hbox{\footnotesize\fnsymbol{footnote}}}

\newdimen\subtitleskip
\def\@makechapterhead#1{%
\vbox{\vskip57pt\parindent \z@  \reset@font
    \interlinepenalty\@M
    \large \bfseries\chaptertitlefont 
\baselineskip17pt
{\chapternumberfont\ifturnoffs\else
\fi\vskip17pt
\hyphenpenalty=10000\relax
{\let\thanks\dofootnotenumber
\let\footnote\dofootnotenumber\vskip1sp
\Large\boldmath\chaptertitlefont\noindent
\uppercase{#1}}
\vskip1sp}
}
\normalsize}


\let\articletitle\chapter
\def\articlesubtitle#1{\vskip-\subtitleskip\vskip9pt{{
\large\baselineskip=16pt\boldmath\articlesubtitlefont
\noindent#1
\vskip1sp}}}

\def\@schapter#1{\if@twocolumn
                   \@topnewpage[\@makeschapterhead{#1}]%
                 \else
                   \@makeschapterhead{#1}%
                   \@afterheading
                 \fi}

\def\@makeschapterhead#1{%
\global\titletrue
  \vspace*{-30pt}%
  {\parindent \z@ \raggedright
    \reset@font
    \interlinepenalty\@M
    \large \bfseries\schaptertitlefont #1\par\nobreak
\vskip12pc
}}

\newcount\c@alphanum
\c@alphanum=0
\newif\ifappendon
\def\appendix#1{\newpage\par
\global\advance\c@alphanum by1
\xdef\applett{\Alph{alphanum}}
\global\appendontrue
\global\c@theorem=0
  \setcounter{chapter}{1}%
  \setcounter{section}{0}%
  \setcounter{figure}{0}%
  \setcounter{table}{0}%
  \setcounter{equation}{0}%
  \renewcommand{\@chapapp}{\appendixname}%
  \renewcommand{\thechapter}{\Alph{chapter}}
\def\one{#1}\ifx\one\empty
\section*{Appendix \ifnum\c@alphanum >1\relax\applett\fi}
\addcontentsline{toc}{section}{Appendix \ifnum\c@alphanum >1\relax\applett\fi}
\else
\section*{Appendix: #1}
\addcontentsline{toc}{section}{Appendix: #1}
\fi
\def\thechapter{\Alph{alphanum}}
\def\theapprunninghead{\Alph{alphanum}}
\markboth{APPENDIX\ \ \theapprunninghead}{APPENDIX\ \ \theapprunninghead}
\small
}

\newcount\c@calphanum

\def\chapappendix#1{\par
\global\advance\c@calphanum by1
\xdef\applett{\the\c@chapter.\Alph{calphanum}}
\global\appendontrue
  \setcounter{theorem}{0}%
  \setcounter{section}{0}%
  \setcounter{figure}{0}%
  \setcounter{table}{0}%
  \setcounter{equation}{0}%
\def\thechapter{\Alph{calphanum}}
\def\one{#1}\ifx\one\empty
\section*{Appendix \ifnum\c@calphanum >1\relax\applett\fi}
\addcontentsline{toc}{section}{Appendix \ifnum\c@calphanum >1\relax\applett\fi}
\else
\section*{Appendix: #1}
\addcontentsline{toc}{section}{Appendix: #1}
\fi
\small\let\@currentlabel=\applett
}

%%% <<=== end Chapter, Part, and Appendix Commands 


%%% Counters ===>>
\newcounter {part}
\newcounter {chapter}
\newcounter {section}[chapter]
\newcounter {subsection}[section]
\newcounter {subsubsection}[subsection]
\newcounter {paragraph}[subsubsection]
\newcounter {subparagraph}[paragraph]

%%% <=== end counters

\newif\ifchapsect
\def\chaptersection{\global\chapsecttrue}

%%% Header Level Names ==>>
\renewcommand{\thepart}         {\Roman{part}}
\renewcommand{\thechapter}      {\arabic{chapter}}
\renewcommand{\thesection}      {\ifchapsect\thechapter.\fi\arabic{section}}
\renewcommand{\thesubsection}   {\thesection.\arabic{subsection}}
\renewcommand{\thesubsubsection}{\thesubsection .\arabic{subsubsection}}
\renewcommand{\theparagraph}    {\thesubsubsection.\arabic{paragraph}}
\renewcommand{\thesubparagraph} {\theparagraph.\arabic{subparagraph}}
\newcommand{\@chapapp}{\chaptername}

%%% <<== End Header Level Names


%%% Section Commands ====>>>>

% \@startsection {NAME}{LEVEL}{INDENT}{BEFORESKIP}{AFTERSKIP}{STYLE}
%            optional * [ALTHEADING]{HEADING}
%    Generic command to start a section.
%    NAME       : e.g., 'subsection'
%    LEVEL      : a number, denoting depth of section -- e.g., chapter=1,
%                 section = 2, etc.
%    INDENT     : Indentation of heading from left margin
%    BEFORESKIP : Absolute value = skip to leave above the heading.
%                 If negative, then paragraph indent of text following
%                 heading is suppressed.
%    AFTERSKIP  : if positive, then skip to leave below heading, else
%                 negative of skip to leave to right of run-in heading.
%    STYLE      : commands to set style
%  If '*' missing, then increments the counter.  If it is present, then
%  there should be no [ALTHEADING] argument.
%  Uses the counter 'secnumdepth' whose value is the highest section
%  level that is to be numbered.
\newcount\temptoc
\newif\ifupperandlower

\def\@sect#1#2#3#4#5#6[#7]#8{\let\dolabelnow\relax%
\ifnum #2>\c@secnumdepth
     \let\@svsec\@empty\else 
     \refstepcounter{#1}\edef\@svsec{\csname the#1\endcsname
\ifchapsect\else\ifnum #2<2.\fi\fi%
\hskip 2em }\fi
     \@tempskipa #5\relax
\ifdim \@tempskipa>\z@ 
%
        \begingroup#6\relax  
          \@hangfrom{\hskip #3\relax\@svsec}{\interlinepenalty \@M 
     \hyphenpenalty=10000 % No hyphenation in section heads
     \raggedright 
     %% make section head uppercase
     \ifnum#2=1
{\def\label##1{\gdef\dolabelnow{\savelabel{##1}}}%
     \global\setbox0=\hbox{\def\\ {\relax}#8}}%
\ifupperandlower\else\uppercase{\fi\fi%
     \def\label##1{}%
     \def\\ {\hfill\break}%
%
\gdef\eight{\ifupperandlower#8\else{#8}\fi}%
\eight
%
\ifnum#2=1\ifupperandlower\else}\fi\fi\par}%%
     \endgroup%
     \csname #1mark\endcsname{#7} % 
     \temptoc=\c@tocdepth \advance\temptoc by1
\ifnum\temptoc >#2
     {%<===
     \def\\ { }%
    \addcontentsline{toc}{#1}{\ifnum #2>\c@secnumdepth\else%
                      \protect\numberline{\csname the#1\endcsname
     \ifnum#2=1.\fi}\fi
                    #7 }%<===
    }%<===
\fi%<===
\else
        \def\@svsechd{#6\hskip #3\relax
                 \@svsec #8.\csname #1mark\endcsname
                   {#7}
{
\addcontentsline{toc}{#1}{\ifnum #2>\c@secnumdepth \else
                             \protect\numberline{\csname the#1\endcsname
     \ifnum#2=1.\fi}\fi
                       #7}}
}%% keep subsubsection contained
\fi\@xsect{#5} 
\ifnum#2=1 \ifsprunningheads\rhead{#7}\fi\fi
\def\@currentlabel{\ifchapsect\else\ifappendon\applett.\fi\fi%
\csname the#1\endcsname}
\dolabelnow %% to make label not be uppercase.
}

\def\@ssect#1#2#3#4#5{%
  \@tempskipa #3\relax
  \ifdim \@tempskipa>\z@
    \begingroup
      #4{%
        \@hangfrom{\hskip #1}%
          \interlinepenalty \@M 
\ifupperandlower\else\ifnum#1<3\relax\uppercase{\fi\fi
#5
\ifupperandlower\else\ifnum#1<3}\fi\fi
\@@par}%
    \endgroup
  \else
    \def\@svsechd{#4{\hskip #1\relax #5}}%
  \fi
  \@xsect{#3}}
\newif\if@afterindent \@afterindenttrue
\def\@afterheading{%
  \@nobreaktrue
  \everypar{%
    \if@nobreak
      \@nobreakfalse
      \clubpenalty \@M
      \if@afterindent \else
        {\setbox\z@\lastbox}%
      \fi
    \else
      \clubpenalty \@clubpenalty
      \everypar{}%
    \fi}}

\newcommand{\section}{\@startsection {section}{1}{\z@}%
                                   {12pt}%
                                   {12pt}%
{\reset@font\large\boldmath\bfseries\sectionfont}}
\newcommand{\subsection}{\@startsection{subsection}{2}{\z@}%
                                     {12pt}%
                                     {12pt}%
  {\reset@font\large\boldmath\bfseries\subsectionfont}}
\newcommand{\subsubsection}{\@startsection{subsubsection}{3}{\z@}%
                                     {12pt}%
                                     {-1em}
  {\reset@font\normalsize\boldmath\bfseries\subsubsectionfont}}
\newcommand{\paragraph}{\@startsection{paragraph}{4}{\z@}%
                                    {12pt}%
                                    {-1em}%
                  {\reset@font\normalsize\bfseries\paragraphfont}}
\newcommand{\subparagraph}{\@startsection{subparagraph}{5}{\parindent}%
                                       {3.25ex \@plus1ex \@minus .2ex}%
                                       {-1em}%
                   {\reset@font\normalsize\bfseries}}
%%% <<=== end section commands


%%% Verse, Quote, Extract ==>>

\newenvironment{verse}
               {\let\\=\@centercr
                \list{}{\itemsep      \z@
                        \itemindent   -1.5em%
                        \listparindent\itemindent
                        \rightmargin  \leftmargin
                        \advance\leftmargin 1.5em}%
                \item[]}
               {\endlist}
\newenvironment{quotation}
               {\vskip1sp\small
\list{}{\listparindent 1.5em%
                        \itemindent    \listparindent
                        \rightmargin   \leftmargin
                        \parsep        \z@ \@plus\p@}%
                \item[]}
               {\endlist}
\newenvironment{quote}
               {\vskip1sp\small
\list{}{\rightmargin\leftmargin}%
                \item[]}
               {\endlist}
\let\extract\quotation
\let\endextract\endquotation
%% <<== end Verse, Quote, Extract 


%%% Math ===>>>
%%%%%%%%%%%%

\def\@eqnnum{\hbox{\@normalsize\rm(\theequation)}} 
\def\xrelax{\relax}
\newif\iffirsttime
\global\firsttimetrue
\newif\ifequation

\def\themathletter{\relax}
\def\mathletter#1{\gdef\themathletter{#1}}
\def\spletter#1{\gdef\thespletter{#1}}
\spletter{}

\def\]{\relax\ifmmode\ifinner\@badmath\else$$\fi%%$$ BRACE MATCH HACK
        \else \@badmath \fi\ignorespaces}

\def\@@eqncr{\let\@tempa\relax 
    \ifcase\@eqcnt \def\@tempa{& & &}\or \def\@tempa{& &} 
      \else \def\@tempa{&}\fi 
     \@tempa 
\ifx\themathletter\xrelax
%% Aug. 9 bug fix ==>
\if@eqnsw
\stepcounter{equation}\fi
%% <==
\else
\ifx\themathletter\xa\global\firsttimetrue\fi
\ifx\themathletter\xA\global\firsttimetrue\fi
\iffirsttime\global\firsttimefalse\stepcounter{equation}
\fi\fi
\if@eqnsw\@eqnnum 
\fi
\ifx\templabel\relax\else\savelabel{\templabel}\fi%%% <<=== oct 14, make label
\global\let\templabel\relax                  % catch current number and letter
\global\let\themathletter\xrelax %% <<=== oct 4
\global\@eqnswtrue\global\@eqcnt\z@\cr}

\def\eqnarray{\expandafter\ifx\csname QQQ\endcsname\relax
\let\@currentlabel=\theequation
\let\label\xlabel
\global\@eqnswtrue
\global\@eqcnt\z@\tabskip\@centering\let\\=\@eqncr
$$\everycr={\noalign{\gdef\themathletter{\relax}}}
\halign to \displaywidth\bgroup\@eqnsel\hskip\@centering
  $\displaystyle\tabskip\z@{##}$&\global\@eqcnt\@ne 
  \hskip 2\arraycolsep \hfil${##}$\hfil
  &%
\global\@eqcnt\tw@ \hskip 2\arraycolsep $\displaystyle\tabskip\z@{##}$\hfil 
   \tabskip\@centering&\llap{##}\tabskip\z@\cr\else\saveeqnarray\fi}

\def\endeqnarray{\@@eqncr\egroup
\global\firsttimetrue\spletter{}$$
\let\@currentlabel=\theequation
\ifx\templabel\relax\else\savelabel{\templabel}\fi%
\global\let\templabel\relax
\gdef\themathletter{\relax}\global\@ignoretrue}

%%% Just in case anyone wants a left aligned eqnarray

\newdimen\mathindent
\mathindent = \parindent

\def\lefteqnarray{\let\@currentlabel=\theequation
\let\label\xlabel
\global\@eqnswtrue
\global\@eqcnt\z@\tabskip\mathindent\let\\=\@eqncr
$$\everycr={\noalign{\gdef\themathletter{\relax}}}
\halign to \linewidth\bgroup\@eqnsel \hskip\@centering
  $\displaystyle\tabskip\z@{##}$&\global\@eqcnt\@ne 
  \hskip 2\arraycolsep \hfil${##}$\hfil  &%
\global\@eqcnt\tw@ \hskip 2\arraycolsep $\displaystyle\tabskip\z@{##}$\hfil 
   \tabskip\@centering&\llap{##}\tabskip\z@\cr}

\let\endlefteqnarray\endeqnarray

\def\leftequation{\let\label\xlabel
\@beginparpenalty\predisplaypenalty
\@endparpenalty\postdisplaypenalty
\trivlist \item[]\leavevmode
 \hbox to\linewidth\bgroup $\m@th% $ TO MAKE DOLLAR NESTING OK
  \displaystyle
\hskip\mathindent}

\def\endleftequation{$\hfil
\displaywidth\linewidth
%
\ifx\themathletter\xrelax\global\firsttimetrue%
\ifx\themathletter\xa\global\firsttimetrue\fi
\ifx\themathletter\xA\global\firsttimetrue\fi
\refstepcounter{equation}\else%
\iffirsttime\global\firsttimefalse\refstepcounter{equation}\fi\fi%
%
\@eqnnum\egroup 
%
\gdef\themathletter{\relax}\global\equationfalse
\global\@ignoretrue
\let\@currentlabel=\theequation
\ifx\templabel\relax\else\savelabel{\templabel}\fi%
\global\let\templabel\relax
%
\endtrivlist}

\let\savelabel\label % Aug 13, '93 bug fix
\def\xlabel#1{\gdef\templabel{#1}}
\let\templabel\relax

\def\equation{\expandafter\ifx\csname QQQ\endcsname\relax%
$$ % $$ BRACE MATCHING HACK
\let\label\xlabel\else\saveequation\fi}

\def\endequation{%
\ifx\themathletter\xa\global\firsttimetrue\fi
\ifx\themathletter\xA\global\firsttimetrue\fi
\ifx\themathletter\xrelax\global\firsttimetrue%
\refstepcounter{equation}\else%
\iffirsttime\global\firsttimefalse\refstepcounter{equation}\fi\fi%
\eqno \@eqnnum % $$ BRACE MATCHING HACK
$$\gdef\themathletter{\relax}\global\equationfalse
\global\@ignoretrue
\let\@currentlabel=\theequation
\ifx\templabel\relax\else\savelabel{\templabel}\fi%
\global\let\templabel\relax}

\@addtoreset{equation}{chapter}

\def\theequation{\thespletter\ifappendon% 
\applett.\arabic{equation}\themathletter\else
\arabic{chapter}.\arabic{equation}%
\themathletter\fi}

\def\nochapequationnumber{
\def\theequation{\thespletter\ifappendon%
\applett.\arabic{equation}\themathletter\else\arabic{equation}%
\themathletter\fi}}

\def\sectionequationnumber{
\@addtoreset{equation}{section}
\def\theequation{\thespletter\ifappendon%
\applett.\arabic{equation}\themathletter\else\the\c@section.\arabic{equation}%
\themathletter\fi}}

%%% <<<=== end Math 

%%% Figure and Table Captions ==>>>
\newcounter{figure}[chapter]

\renewcommand{\thefigure}{\ifturnoffs\else%
\ifappendon\applett.\else\thechapter.\fi\fi%
\@arabic\c@figure\theletter}
\def\fps@figure{tbp}
\def\ftype@figure{1}
\def\ext@figure{lof}
\def\fnum@figure{\figurename~\thefigure}
\newenvironment{figure}
               {\@float{figure}}
               {\end@float}
\newenvironment{figure*}
               {\@dblfloat{figure}}
               {\end@dblfloat}
\newcounter{table}[chapter]
\renewcommand{\thetable}{\ifturnoffs\else%
\ifappendon\applett.\else\thechapter.\fi\fi%
\@arabic\c@table\theletter}
\def\fps@table{tbp}
\def\ftype@table{2}
\def\ext@table{lot}
\def\fnum@table{\tablename~\thetable}
\newenvironment{table}
               {\@float{table}}
               {\end@float}
\newenvironment{table*}
               {\@dblfloat{table}}
               {\end@dblfloat}
\newlength\abovecaptionskip
\newlength\belowcaptionskip
\setlength\abovecaptionskip{10\p@}
\setlength\belowcaptionskip{0\p@}

\def\xfigure{figure}
\long\def\@makecaption#1#2{%
\vskip\abovecaptionskip
\ifx\@captype\xfigure
\sbox\@tempboxa{{\fignumfont#1.%
\let\it\captionitalic
\quad \figtextfont\let\newline\ #2}}%
  \ifdim \wd\@tempboxa >.9\hsize
  \fignumfont #1%
\ifcontinuedcaption\global\continuedcaptionfalse\else.%
\quad\fi
  \figtextfont #2
  \else
    \global \@minipagefalse
    \hbox to\hsize{\hfil\box\@tempboxa\hfil}%
  \fi
\vskip\belowcaptionskip
\else %% table caption
{\fignumfont#1%
\ifcontinuedcaption\global\continuedcaptionfalse\else.%
\quad\fi\tabtextfont #2\par}% 
\vskip2pt
\fi
\xdef\@currentlabel{\expandafter\csname p@\@captype\endcsname%
\expandafter\csname the\@captype\endcsname}
%
\gdef\theletter{\relax}%%
\global\continuedcaptionfalse
}

\let\savetable\table

\def\table{\small\savetable}

%%% code to get text in tables to extend all the way to left and right
%%% of table

\def\xtable{table}
\def\@array[#1]#2{\setbox\@arstrutbox=\hbox{\vrule 
     height\arraystretch \ht\strutbox
     depth\arraystretch \dp\strutbox
     width\z@}\@mkpream{#2}\edef\@preamble{\halign \noexpand\@halignto
\bgroup%
\tabskip\z@\@arstrut\@preamble
\ifx\@captype\xtable\hskip-\tabcolsep\fi%% <==== Changed
\tabskip\z@ \cr}%
\let\@startpbox\@@startpbox \let\@endpbox\@@endpbox%
  \if #1t\vtop \else \if#1b\vbox \else \vcenter \fi\fi%
  \bgroup\let\par\relax%
  \let\@sharp##\let\protect\relax \lineskip\z@\baselineskip\z@\@preamble}

%% new
\def\new@tabacol{\edef\@preamble{\@preamble\hskip0pt}}

\def\@tabclassz{\ifcase \@lastchclass\@acolampacol% 
\or \@ampacol \or
   \or \or \@addamp \or \@acolampacol\or \@firstampfalse 
\ifx\@captype\xtable \new@tabacol\else\@tabacol \fi%
\fi%
\edef\@preamble{\@preamble%
  \ifcase \@chnum%
     \hfil\ignorespaces\@sharp\unskip\hfil%
     \or \ignorespaces\@sharp\unskip\hfil%
     \or \hfil\hskip\z@ \ignorespaces\@sharp\unskip\fi}}

\let\savetabular\tabular
\def\tabular{\tablefontsize\baselineskip=12pt\savetabular}
\@namedef{tabular*}#1{\def\@halignto{to#1}\tablefontsize\baselineskip12pt\@tabular}

\let\savehline\hline
\def\sphline{\noalign{\vskip3pt}\savehline\noalign{\vskip3pt}}

\def\tablenotes{\bgroup\parfillskip=0pt plus 1fil
\leftskip=0pt\relax \rightskip=0pt
\vskip2pt\footnotesize}
\def\endtablenotes{\vskip1pt\egroup}

%%%%%%%%%

\def\widefigure{\clearpage
\advance\oddsidemargin-88pt
\advance\evensidemargin-88pt
\bgroup
\global\textwidth=\textheight 
\hsize=\textheight
\textheight=28.5pc
\thispagestyle{empty}\def\@captype{figure}}
\def\endwidefigure{\vskip1sp\egroup\clearpage
\global\textwidth=\savetextwidth
\advance\oddsidemargin88pt
\advance\evensidemargin88pt
}

\def\landscapemode{\clearpage
\advance\oddsidemargin-88pt
\advance\evensidemargin-88pt
\bgroup
\global\textwidth=\textheight 
\hsize=\textheight
\textheight=28.5pc
\thispagestyle{empty}}
\def\endlandscapemode{\vskip1sp\egroup\clearpage
\global\textwidth=\savetextwidth
\advance\oddsidemargin88pt
\advance\evensidemargin88pt
}

\def\widetable{\clearpage
\advance\oddsidemargin-88pt
\advance\evensidemargin-88pt
\bgroup\global\textwidth=\textheight 
\hsize=\textwidth
\thispagestyle{empty}
\bgroup\def\@captype{table}
\centering
}
\def\endwidetable{\vskip1sp\egroup
\vskip1sp\egroup
\clearpage
\global\textwidth=\savetextwidth
\advance\oddsidemargin88pt
\advance\evensidemargin88pt
}
%
%% ==> new
\newdimen\belowtableskip
\newdimen\abovetableskip
\abovetableskip=6pt
\def\xtable{table}

\long\def\@makenarrowcaption#1#2{%
\vskip-12pt
\hbox to\textwidth{\hss\vbox to 0pt{\vss
\hsize=1.5in\parskip=6pt\footnotesize%
{\fignumfont\relax #1}\hskip1em\figtextfont\relax #2}}}

\def\narrowcaption{\let\@makecaption\@makenarrowcaption
\@ifnextchar[\@xcaption\@ycaption}
\def\@xcaption[#1]#2{\caption[#1]{#2}}
\def\@ycaption#1{\caption{#1}}


\def\dblcaption#1#2{
\hbox to\textwidth{%
\vtop{\textwidth=13.5pc\hsize13.5pc\caption{#1}}\hfill\vtop{\textwidth=13.5pc%
\hsize13.5pc\caption{#2}}}
}

\def\lettereddblcaption#1#2#3#4{
\hbox to\textwidth{%
\vtop{\textwidth13.5pc \hsize13.5pc\letteredcaption{#1}{#2}}\hfill%
\vtop{\textwidth13.5pc \hsize13.5pc\letteredcaption{#3}{#4}}}
}

%%%%%%%%%
%%%% Continued captions

\def\Nfnum@figure{Figure \thefigure\ {(continued)}\hfill\break}

\def\Nfnum@table{Table \thetable\ {(continued)}\hfill\break}

\def\Ccaption{\@dblarg{\C@caption\@captype}}

\long\def\C@caption#1[#2]#3{\par
\begingroup
    \@parboxrestore
    \normalsize
    \@makecaption{\csname fnum@#1\endcsname}{\ignorespaces#3}\par
  \endgroup}

\newif\ifcontinuedcaption
\def\contcaption{\global\continuedcaptiontrue
\let\fnum@figure\Nfnum@figure
\let\fnum@table\Nfnum@table
\global\advance\expandafter\csname c@\@captype\endcsname-1
\@ifnextchar[\xCcaption\yCcaption}

\def\xCcaption[#1]#2{\normalsize\caption[\hskip-14pt(continued) #1]{#2}}

\def\yCcaption#1{{\normalsize\caption[\hskip-14pt(continued)]{#1}}}

\def\narrowcontcaption{\global\continuedcaptiontrue
\let\@makecaption\@makenarrowcaption
\let\fnum@figure\Nfnum@figure
\let\fnum@table\Nfnum@table
\global\advance\expandafter\csname c@\@captype\endcsname-1
\@ifnextchar[\xCcaption\yCcaption}


%%%%%%%%% Lettered Captions

\def\theletter{\relax}
\newif\ifxfirsttime
\xfirsttimetrue
\def\xa{a}
\def\xA{A}

\def\caption{\ifx\@captype\xtable
\ifx\theletter\xrelax
\global\xfirsttimetrue\refstepcounter\@captype\else
\ifx\theletter\xa \global\xfirsttimetrue\fi
\ifx\theletter\xA \global\xfirsttimetrue\fi
\ifxfirsttime\global\xfirsttimefalse\refstepcounter\@captype\fi\fi
\else
\ifx\theletter\xrelax
\global\xfirsttimetrue\refstepcounter\@captype\else
\ifx\theletter\xa \global\xfirsttimetrue\fi
\ifx\theletter\xA \global\xfirsttimetrue\fi
\ifxfirsttime\global\xfirsttimefalse\refstepcounter\@captype\fi\fi\fi
\edef\@currentlabel{\expandafter\csname p@\@captype\endcsname%
\expandafter\csname the\@captype\endcsname}
\xdef\currcaptype{\@captype}
\@dblarg{\@caption\@captype}}

\def\letteredcaption#1{\gdef\theletter{#1}\caption}

%%%%%%%%%

\def\fnum@figure{Figure\ \thefigure}
\def\fnum@table{Table\ \thetable}

%%% <<=== end Figure and Table Captions


%%% Table of Contents, List of Figures, LOT ===>>

\newcommand{\@pnumwidth}{1.55em}
\newcommand{\@tocrmarg} {2.55em}
\newcommand{\@dotsep}{4.5}

\newcommand{\tableofcontents}{%
\ifpreface\global\prefacefalse\else
\setcounter{page}{5}\fi
    \chapter*{{\toctitlefont\contentsname}
        \@mkboth{\uppercase{\contentsname}}{\uppercase{\contentsname}}}%
\rightrunninghead{\contentsname}
{\tocarticletitlefont
\baselineskip=9pt
\vskip1sp
\vskip-14pt
    \@starttoc{toc}%
    \if@restonecol\twocolumn\fi}
\newpage
\markboth{}{}
    }

\newcommand{\sptableofcontents}{%
    \chapter*{{\toctitlefont\contentsname}
        \@mkboth{\uppercase{\contentsname}}{\uppercase{\contentsname}}}%
\rightrunninghead{\contentsname}
{\tocarticletitlefont
\baselineskip=9pt
\vskip1sp
\vskip-14pt
    \@starttoc{toc}%
    \if@restonecol\twocolumn\fi}
\newpage
\markboth{}{}
    }
\newskip\tocpartskip
\tocpartskip=8pt plus1pt
\def\l@part#1#2{%
    \vskip\tocpartskip
    \begingroup
      \setlength\@tempdima{3em}%
      \parindent \z@ \rightskip \@pnumwidth
      \parfillskip -\@pnumwidth
      {\leavevmode
\vskip1pt
        \tocarticletitlefont 
Part #1\hfil \hbox to\@pnumwidth{\hss %#2
}}\par
       \nobreak
         \global\@nobreaktrue
         \everypar{\global\@nobreakfalse\everypar{}}
    \endgroup
}

\def\l@chapter#1#2{%
      \penalty-\@highpenalty
    \vskip 8pt 
     \setlength\@tempdima{1.5em}%
    \begingroup
      \parindent \z@ \rightskip \@pnumwidth
      \parfillskip -\@pnumwidth
      \leavevmode\tocarticletitlefont%
      \advance\leftskip\@tempdima
      \hskip -\leftskip
\hfill #1\nobreak\hfill\hbox to\@pnumwidth{\hss #2}\par
      \penalty\@highpenalty
    \endgroup
}

\def\l@spchapter#1#2{%
      \penalty-\@highpenalty
    \vskip -3pt 
     \setlength\@tempdima{1.5em}%
    \begingroup
      \parindent \z@ \rightskip \@pnumwidth
      \parfillskip -\@pnumwidth
      \leavevmode\tocarticletitlefont%
      \advance\leftskip\@tempdima
      \hskip -\leftskip
\hfill #1\nobreak\hfill\hbox to\@pnumwidth{\hss #2}\par
      \penalty\@highpenalty
    \endgroup
}

\def\l@index#1#2{%
      \penalty-\@highpenalty
     \setlength\@tempdima{1.5em}%
    \begingroup
      \parindent \z@ \rightskip \@pnumwidth
      \parfillskip -\@pnumwidth
      \leavevmode\tocarticletitlefont%
      \advance\leftskip\@tempdima
      \hskip -\leftskip
#1\nobreak\hfill\hbox to\@pnumwidth{\hss #2}\par
      \penalty\@highpenalty
    \endgroup
}



% \@dottedtocline{LEVEL}{INDENT}{NUMWIDTH}{TITLE}{PAGE} :
%   Macro to produce a table of contents line with the following
%   parameters:
%     LEVEL    : If LEVEL > \c@tocdepth, then no line produced.
%     INDENT   : Total indentation from the left margin.
%     NUMWIDTH : Width of box for number if the TITLE has a
%                \numberline command.
%                As of 25 Jan 88, this is also the amount of extra indentation
%                added to second and later lines of a multiple line entry.
%     TITLE    : Contents of entry.
%     PAGE     : Page number.
%
%  Uses the following parameters, which must be set by the document style.
%  They should be defined with \def's.
%    \@pnumwidth : Width of box in which page number is set.
%    \@tocrmarg  : Right margin indentation for all but last line of
%                  multiple-line entries.
%    \@dotsep    : Separation between dots, in mu units.  Should be \def'd to
%                  a number like 2 or 1.7
%
\def\@dottedtocline#1#2#3#4#5{%
\ifnum #1>\c@tocdepth \else 
%%
\vskip1pt %% <== bug fix, was before the \if. 10/20/97
%%
\vskip \z@ plus.2\p@ {\leftskip #2\relax \rightskip \@tocrmarg \parfillskip -\rightskip \parindent #2\relax 
\@afterindenttrue \interlinepenalty \@M \leavevmode \@tempdima #3\relax 
\advance \leftskip \@tempdima \hbox {}\hskip -\leftskip #4%
\nobreak% \leaders \hbox {$\m@th \mkern
% \@dotsep mu.\mkern \@dotsep mu$}
\hfill\nobreak\hbox to\@pnumwidth% 
{\hss\reset@font\tocarticletitlefont #5}\par}\fi}


\def\l@section{\vskip1pt\ifnum\c@tocdepth>2
\tocarticletitlefont\vskip2pt\setlength\@tempdima{18pt}\fi%
\@dottedtocline{1}{1.5em}{3em}}

\def\l@author{}
\def\l@subsection{\@dottedtocline{2}{4.5em}{3.2em}}
\def\l@subsubsection{\@dottedtocline{3}{4.5em}{4.5em}}
\def\l@paragraph{\@dottedtocline{4}{10em}{5em}}
\def\l@subparagraph{\@dottedtocline{5}{12em}{6em}}

\def\listoffigures{\@restonecolfalse\if@twocolumn\@restonecoltrue\onecolumn\fi
\chapter*{\toctitlefont List of Figures}
{\markboth{List of Figure}{List of Figures}}
\addcontentsline{toc}{spchapter}{\protect\cnumberline{}List of Figures}
\renewcommand{\@tocrmarg} {4.55em}%% was 2.55em
\@starttoc{lof}\if@restonecol \twocolumn\fi}

\def\l@figure{\@dottedtocline{0}{1.5em}{2.8em}}

\def\listoftables{\@restonecolfalse\if@twocolumn\@restonecoltrue\onecolumn
 \fi\chapter*{{\toctitlefont List of Tables}}
{\markboth{List of Tables}{List of Tables}}
\renewcommand{\@tocrmarg} {4.55em}%% was 2.55em
\@starttoc{lot}\if@restonecol
 \twocolumn\fi
 \addcontentsline{toc}{spchapter}{\protect\cnumberline{}List of Tables}}
\let\l@table\l@figure

%%% <<=== end TOC, LOT, LOF

%%% Bibliography, References, Acknowledgements ===>>

% Make \cite yield citation in parenthesis or square brackets.
% and put a dash between sequential numbers as long as they
% start with the first number.

% +++
\newif\iflookfurther
\def\lookforcommas#1,#2,#3{\def\three{#3}\ifx\three\empty%
\global\lookfurtherfalse\else\global\lookfurthertrue\fi}

\newcount\firstcitenum
\newcount\secondcitenum
\newcount\lastcitenum
\newcount\testcitenum

\def\dolastnumber{\relax}
\def\@cite#1#2{\hskip1sp\relax%<== April 5, 1995, to let it break over lines.
%% below, test to see if second arg is empty necessary to avoid
%% comma and space at end of some citations. sept 29, 2003
\lcitebracket#1\if@tempswa\def\two{#2}\ifx\two\empty\else , #2\fi\fi
\iflookfurther\ifnum\firstcitenum>0\the\firstcitenum\fi\dolastnumber
\global\lookfurtherfalse\fi\rcitebracket%
\global\firstcitenum=0\relax%
\global\secondcitenum=0\relax%
\global\lastcitenum=0\relax%
}

\def\lcitebracket{}\def\rcitebracket{} % default is no brackets

\def\kluwerbib{
\def\thebibliography##1{\notes\newpage\chapter*{References}
\gdef\rrunninghead{REFERENCES}
\addcontentsline{toc}{chapter}{\protect\cnumberline{}References}
\everypar={}
\bgroup
\frenchspacing
\normalsize
\parindent=-1pc
\leftskip=1pc
\vskip1sp
}

\def\endthebibliography{\vskip24pt\egroup\parindent=0pt\relax
\newpage}

\def\chapreferences{\chapthebibliography{}}
\def\endchapreferences{\endchapthebibliography}

\def\chapthebibliography##1{\notes\section*{References}
%\gdef\rrunninghead{REFERENCES}
\addcontentsline{toc}{section}{References}
\everypar={}
\bgroup
\frenchspacing
\normalsize
\parindent=-1pc
\leftskip=1pc
\vskip1sp
}

\def\endchapthebibliography{\vskip24pt\egroup\parindent=0pt\relax}

\def\references{\thebibliography{}}
\def\endreferences{\endthebibliography}

\def\newblock{}

\def\lcitebracket{}\def\rcitebracket{}

\def\@citex[##1]##2{%
\if@filesw\immediate\write\@auxout{\string\citation{##2}}\fi
  \let\@citea\@empty
  \@cite{\@for\@citeb:=##2\do
    {\@citea\def\@citea{,%;
\penalty\@m\ }%
     \def\@tempa####1####2\@nil%
{\edef\@citeb{\if####1\space####2\else####1####2\fi}}%
     \expandafter\@tempa\@citeb\@nil
     \@ifundefined{b@\@citeb}{{\reset@font\bf ?}\@warning
       {Citation `\@citeb' on page \thepage \space undefined}}%
\expandafter\csname b@\@citeb\endcsname}}{##1}}


%\def\@bibitem##1{\par}

\def\@bibitem##1{\par\if@filesw \immediate\write\@auxout
       {\string\bibcite{##1}{##1}}\fi\ignorespaces}

\def\@lbibitem[##1]##2{\par\indent%%<== added March 1, 2000
\if@filesw
{\def\protect####1{\string ####1\space}%
\immediate\write\@auxout{\string\bibcite{##2}{##1}}}\fi\ignorespaces}

} % <======== end kluwerbib

%% default bib style, as opposed to \normallatexbib.
\kluwerbib

%%%%==>>

\def\normallatexbib{
\def\lcitebracket{[}\def\rcitebracket{]} % default is no brackets

\def\thebibliography##1{\newpage\chapter*{References}
\gdef\rrunninghead{REFERENCES}
\addcontentsline{toc}{chapter}{\protect\cnumberline{}References}
\everypar={}
      \list{\@biblabel{\arabic{enumiv}}}%
           {\settowidth\labelwidth{\@biblabel{##1}}%
            \leftmargin\labelwidth
            \advance\leftmargin\labelsep
            \if@openbib
              \advance\leftmargin\bibindent
              \itemindent -\bibindent
              \listparindent \itemindent
              \parsep \z@
            \fi
            \usecounter{enumiv}%
            \let\p@enumiv\@empty
            \renewcommand{\theenumiv}{\arabic{enumiv}}}%
      \if@openbib
        \def\newblock{\par}
      \else
        \def\newblock{\hskip .11em \@plus.33em \@minus.07em}%
      \fi
      \sloppy\clubpenalty4000\widowpenalty4000%
      \sfcode`\.=\@m
\frenchspacing
\normalsize
}

\def\endthebibliography{\vskip24pt
\parindent=0pt\relax
     \def\@noitemerr
       {\@latex@warning{Empty `thebibliography' environment}}%
      \endlist
\newpage}

\def\chapthebibliography##1{\notes\section*{References}
%\gdef\rrunninghead{REFERENCES}
\addcontentsline{toc}{section}{References}
\everypar={}
      \list{\@biblabel{\arabic{enumiv}}}%
           {\settowidth\labelwidth{\@biblabel{##1}}%
            \leftmargin\labelwidth
            \advance\leftmargin\labelsep
            \if@openbib
              \advance\leftmargin\bibindent
              \itemindent -\bibindent
              \listparindent \itemindent
              \parsep \z@
            \fi
            \usecounter{enumiv}%
            \let\p@enumiv\@empty
            \renewcommand{\theenumiv}{\arabic{enumiv}}}%
      \if@openbib
        \def\newblock{\par}
      \else
        \def\newblock{\hskip .11em \@plus.33em \@minus.07em}%
      \fi
      \sloppy\clubpenalty4000\widowpenalty4000%
      \sfcode`\.=\@m
\bgroup
\frenchspacing
\normalsize
\itemsep=-1pt
\vskip-12pt
\vskip1sp
\item[]
}

\def\endchapthebibliography{\vskip24pt\egroup\parindent=0pt\relax
     \def\@noitemerr
       {\@latex@warning{Empty `thebibliography' environment}}%
      \endlist
}

\newif\ifsqbracketbib
\def\@lbibitem[##1]##2{\item[\@biblabel{##1}\hfill]\if@filesw
\immediate\write\@auxout{\string\global\string\sqbracketbibtrue}
      {\let\protect\noexpand
       \immediate
       \write\@auxout{\string\bibcite{##2}{##1}}}\fi
\ignorespaces}

\def\@bibitem##1{\item\if@filesw \immediate\write\@auxout
       {\string\bibcite{##1}{\the\value{\@listctr}}}\fi\ignorespaces}

\let\citation\@gobble
\ifll
%% test to see if babel is used:
\ifx\LdfInit\@undefined 
\DeclareRobustCommand\cite{%
  \@ifnextchar [{\@tempswatrue\@citex}{\@tempswafalse\@citex[]}}
\def\@citex[##1]##2{%
 \let\@citea\@empty 
  \@cite{\@for\@citeb:=##2\do
    {\@citea\def\@citea{,\penalty\@m\ }%
     \edef\@citeb{\expandafter\@firstofone\@citeb}%
     \if@filesw\immediate\write\@auxout{\string\citation{\@citeb}}\fi%
     \@ifundefined{b@\@citeb}{\mbox{\reset@font\bfseries ?}%
       \G@refundefinedtrue
       \@latex@warning
         {Citation `\@citeb' on page \thepage \space undefined}}%
       {%\hbox{\csname b@\@citeb\endcsname}
\ifsqbracketbib
\csname b@\@citeb\endcsname
\else
%% don't bother parsing if it is only one
\setbox0=\hbox{\def\two{##2}\expandafter\lookforcommas\two{},{},{},}%+++
%% more than one, need to parse
\iflookfurther%
\let\@citea\@empty%
%% num can be first in sequence, mid-sequence, or end-sequence
%  print first number, save it as firstcitenum
%  check to see if second is sequential
%   yes, save as secondcitenum, continue to see if there is another in sequence
%    no, print firstcitenum and comma,
%        name secondcitenum  see if is starting new sequence
% 
%  etc. 
% print when first number or whenever current number is out of sequence
%
% 
\gdef\dolastnumber{\relax}%
\ifnum\firstcitenum=0
  %% save to compare
  \expandafter\expandafter\global\firstcitenum\csname b@\@citeb\endcsname% 
  %% print
\relax\the\firstcitenum\relax% 
\else%
  \expandafter\testcitenum=\csname b@\@citeb\endcsname%
  \advance\testcitenum by-1\relax
%% start from lastcitenum and work down:
%LAST
   \ifnum\testcitenum=\lastcitenum
    %% sequence of more than three numbers, but may be more in the sequence   
    %% don't do anything until there is a mismatching number
    \expandafter\expandafter\global\lastcitenum\csname b@\@citeb\endcsname% 
    \xdef\dolastnumber{--\the\lastcitenum}%
\else%
%SECOND
      \ifnum\testcitenum=\secondcitenum
       %% sequence of 3 numbers, but maybe not end of sequence
       %% set lastcitenum to current cite num
       \expandafter\expandafter\global\lastcitenum\csname b@\@citeb\endcsname% 
       \xdef\dolastnumber{--\the\lastcitenum}%
\else%
%FIRST
          \ifnum\testcitenum=\firstcitenum
          %% sequence of 2 numbers
   \expandafter\expandafter\global\secondcitenum\csname b@\@citeb\endcsname% 
   %% don't print unless this is the end, (after other numbers) in which case
   %% \dolastnumber will print it
       \xdef\dolastnumber{, \the\secondcitenum}%
\else%
%NOT IN SEQUENCE
\ifnum\lastcitenum>0 --\the\lastcitenum\else%
\ifnum\secondcitenum>0, \the\secondcitenum\fi%
\fi%
\expandafter\expandafter\global\firstcitenum\csname b@\@citeb\endcsname% 
, \the\firstcitenum%
%% reset
    \global\secondcitenum=0\relax%
    \global\lastcitenum=0\relax%
         \fi%
      \fi%
   \fi%
\fi%
\else%
%% this is only used if we don't need to parse
\def\@citea{,\penalty\@m\ }%
\expandafter\csname b@\@citeb\endcsname%
\fi% end test if lookfurther
\fi% end test if sqbracketbib
%%%
}}}{##1}{}}
%%
\else
%% babel used, use simpler definition of cite:
\def\cite{\@ifnextchar [{\@tempswatrue\@citex}{\@tempswafalse\@citex[]}}
\fi
\else % LaTeX 2.09
\def\cite{\@ifnextchar [{\@tempswatrue\@citex}{\@tempswafalse\@citex[]}}

\def\@citex[##1]##2{\if@filesw\immediate\write\@auxout{\string\citation{##2}}\fi
  \let\@citea\@empty
  \@cite{\@for\@citeb:=##2\do
    {\@citea\def\@citea{,\penalty\@m\ }%
     \def\@tempa####1####2\@nil{\edef\@citeb{\if####1\space####2\else####1####2\fi}}%
     \expandafter\@tempa\@citeb\@nil
     \@ifundefined{b@\@citeb}{{\reset@font\bf ?}\@warning
       {Citation `\@citeb' on page \thepage \space undefined}}%
     \expandafter\csname b@\@citeb\endcsname}}{##1}}
\fi
\let\bibdata=\@gobble
\let\bibstyle=\@gobble
\def\bibliography##1{%
  \if@filesw
    \immediate\write\@auxout{\string\bibdata{##1}}%
  \fi
  \@input{\jobname.bbl}}
\def\bibliographystyle##1{%
  \ifx\@begindocumenthook\@undefined\else
    \expandafter\AtBeginDocument
  \fi
    {\if@filesw
       \immediate\write\@auxout{\string\bibstyle{##1}}%
     \fi}}
\expandafter\let\csname b@*\endcsname\@empty

\def\references{\thebibliography{10}
\small
\item[]
}

\def\endreferences{\endthebibliography}

\def\chapreferences{\chapthebibliography{10}
\item[]
}

\def\endchapreferences{\endchapthebibliography}

}

\let\alphabib\normallatexbib
%% <===

%%%%%% Chapter Bibliography Using BibTeX

\def\chapbblname#1{\def\currentfilename{#1}}

\def\chapbibliography#1{%
\if@filesw \immediate \write \@auxout {\string \bibdata {#1}}\fi 
\expandafter\ifx\csname currentfilename\endcsname\relax
\typeout{^^J^^J As soon as your .bbl file is written,
^^J^^J
!! PLEASE WRITE \string\chapbblname\string{<Your .bbl File Name>\string} !!
^^J\space\space\space before \string\chapbibliography{#1}
^^J^^J
The BibTeX Chapter Bibliography, \string\chapbibliography^^J 
\space\space\space Will not print unless you supply this file name!
^^J^^J
}
\else
{\let\thebibliography\chapthebibliography
\let\endthebibliography\endchapthebibliography
\@input {\currentfilename.bbl}}\fi
}

%%%%%%%%%%%%

\def\acknowledgments{\section*{Acknowledgments}
\addcontentsline{toc}{spchapter}{\protect\cnumberline{}Acknowledgments}
}
\def\acknowledgment{\section*{Acknowledgment}
\addcontentsline{toc}{spchapter}{\protect\cnumberline{}Acknowledgment}
}

\def\endacknowledgment{}
\def\endacknowledgments{}


%% end Bibliography, References and Acknowledgments


%%% Footnotes ===>>>

  \renewcommand{\footnoterule}{%
  \kern-3\p@
  \hrule width .4\columnwidth
  \kern 2.6\p@}

\let\savefootnoterule\footnoterule

  \let\footnoterule\relax

\long\def\@makefntext#1{%
    \parindent 1em%
    \noindent
    \@makefnmark#1}

\newcount\footnum
\newcount\savefootnum
\long\def\endnote#1{\global\advance\footnum by 1\relax$^{\the\footnum}$%
\long\expandafter\gdef\csname foot\the\footnum\endcsname{%
{\footnotesize\hsize=\textwidth%\leftskip=12pt\relax\parindent=-12pt
\indent\hbox to12pt{\the\footnum.\hfill}{#1\strut\vskip1sp}}\vskip1pt}}

\let\footnote\endnote

\def\notes{\ifnum\footnum=0\else
\section*{Notes}
\savefootnum=\footnum
\footnum=0
\noindent\loop\ifnum\savefootnum>\footnum%
\global\advance\footnum by1\relax%
\csname foot\the\footnum\endcsname\relax%
\expandafter\gdef\csname foot\the\footnum\endcsname{\relax}\relax%
\repeat\global\footnum=0\relax\fi}

%%% <<<=== End Footnotes


%%% Running heads ===>>>

%% unless we need these, leave these uncommented
    \let\@mkboth\@gobbletwo
    \let\chaptermark\@gobble
    \let\sectionmark\@gobble

\def\ps@headings{\let\@mkboth\markboth
 \def\@oddfoot{\iftitle\global\titlefalse\fi}
\let\@evenfoot\@oddfoot
% \def\@oddfoot{\iftitle\vbox to0pt{%
%\vskip12pt
%\hbox to\textwidth{\hfill\foliofont\thepage\hfill}\vskip1sp\vss}%
%\global\titlefalse\fi\hfill}
% \def\@evenfoot{\iftitle\vbox to0pt{%
%\vskip12pt
%\hbox to\textwidth{\hfill\foliofont\thepage\hfill}%
%\vskip1sp\vss}\global\titlefalse\fi\hfill}
 \def\@evenhead{\iftitle\hfill\else{\foliofont \thepage}\hfill%
\runningheadfont\lrunninghead\fi}
 \def\@oddhead{\iftitle\hfill\else{\runningheadfont\rrunninghead}%
\hfill\foliofont\thepage\fi}%
}

\markboth{}{}

\ps@headings

\pagestyle{headings}
\pagenumbering{roman}



%% Special use fonts ===>>

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% Computer Modern font calls. These will be redefined
%% if PostScript fonts are used.
%%
%% \documentstyle[edbkps]{kapedbk} will use PostScript fonts.
%% \documentstyle{kapedbk} will use Computer Modern fonts.

%% Times-Roman
\def\timesroman{cmr10 }

%% Times-Bold
\def\timesbold{cmbx10 }

%% Times-Italic
\def\timesitalic{cmti10 }

%% Times-BoldItalic
\def\timesbolditalic{cmbx10 }

%% Helvetica
\def\helvetica{cmss10 }
\def\helveticamedium{cmss10 }
\def\helveticabold{cmssbx10 }
\def\helveticaoblique{cmsl10 }
\def\helveticaboldoblique{cmsl10 }

%% Special Use Fonts ==>>

%% Table of Contents
  \font\toctitlefont=\timesbold at 15pt % title of Table of Contents
  \font\tocbodyfont=\timesroman at 10pt % text of table of contents
  \font\tocauthorfont=\timesitalic at 10pt % used for introduction author
  \font\tocarticletitlefont=\timesroman at 10pt %


%% Running heads
  \font\runningheadfont=\timesitalic at10pt
  \font\foliofont=\timesroman at 11pt

%% Book Title, Half title page
  \font\halftitlefont=\helveticamedium at 18pt
  \font\booktitlefont=\helveticamedium at 18pt
  \font\subtitlefont=\helvetica at 12pt
  \font\seriestitlefont=\helveticamedium at 12pt
  \font\professorfont=\helvetica at 8pt
  \font\seriesbookfont=\helveticamedium at 8pt

  \font\editorfont=\timesroman at 11pt %% specs call for \helvetica
  \font\editoraffilfont=\helvetica at 9pt%% specs call for \helveticamedium

  \font\authorfont=\timesroman at 11pt %% specs call for \helvetica
  \font\kluwerfont=\helveticabold at 9pt %% specs call for \helveticamedium
  \font\locationfont=\helvetica at 9pt

  \font\copyrightfont=\helveticamedium at 6pt
  \font\copyrighttextfont=\helvetica at 6pt
  \font\dedicationfont=\timesbolditalic at 10pt

%% Part Title
  \font\partfont=\timesbold at 16pt %\helvetica at 24pt
  \font\partnumberfont=\timesbold at 12pt %\helvetica at 42pt
  \font\partsubtitle=\timesbolditalic at 16pt %\helveticaboldoblique at 24pt

  \font\affilfont=\timesitalic at 9pt %% 
  \font\emailfont=\helvetica at 8pt %% all caps


%% Chapter Title 
  \font\chapternumberfont=\timesroman at 15pt
  \font\chaptertitlefont=\timesbold at 15pt
  \font\schaptertitlefont=\timesbold at 15pt
  \font\articlesubtitlefont=\timesitalic at 15pt

%% Section heads
  \font\sectionfont=\timesbold at 13pt
  \font\subsectionfont=\timesbold at 13pt
  \font\subsubsectionfont=\timesbold at 11pt
  \font\paragraphfont=\timesbold at 11pt

%% Theorem fonts
  \font\theoremfont=cmcsc10 at 11pt

%% Caption 
  \font\fignumfont=\timesitalic at 9pt
  \def\figtextfont{\small} %\timesroman at 9pt
  \def\tabtextfont{\small} %\timesroman at 9pt

%% Table fonts
  \def\tabletitlefont{\timesroman}
  \def\tablefontsize{\small}
  \def\tablefootnotesize{\footnotesize}

%% References/Bibliography
\def\bibfontsize{\small}

%% Text fonts
\font\bit=\timesbolditalic at 10pt

%% Fake Small Caps
\font\bigscfont\timesroman at 9.5pt
\font\smscfont\timesroman at 7.5pt

%%% <=== end special use fonts

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%% Fake Small Caps

% To use: \fakesc some stuff*
%% \bigscfont, \smscfont are listed in font section above.

\def\fakesc#1*{\edef\stuff{#1 }%
\expandafter\pickupnewword\stuff* }
\def\saveast{*}

\def\rrelax{\relax}

\def\pickupnewlett#1{\def\letter{#1}%
\ifx\letter\saveast\unskip% 
\let\go\relax \else%  
\ifx\letter\rrelax{\bigscfont\ }%
\let\go\pickupnewword\else% 
\uppercase{\def\ucletter{#1}%  upper case letter
\ifx\letter\ucletter%
{\bigscfont#1}\else{\smscfont#1}\fi}%% 
\let\go\pickupnewlett\fi\fi\go}

\def\pickupnewword#1 {\expandafter\pickupnewlett#1\relax}
%% <== end fakesc

%% Algorithm

{\obeylines

\gdef\looker{\ifx\next\lineending\vskip-1sp%
\noindent\else%
\ifx\next\end\vskip-1sp\let^^M\ \else%
\vskip0pt\noindent\fi\fi}

\gdef\lineending{\parfillskip=0pt plus1fil\relax\futurelet\next\looker}

\gdef\algorithm{\medskip
\def\note##1{\parfillskip=0pt\hfill##1}\def\ {\quad}%
\baselineskip=12pt\parskip=2pt\obeylines\let^^M=\lineending}


\gdef\endalgorithm{\medskip}

\gdef\algorithm{\bgroup\obeylines\obeyspaces\def\ {\quad}
\def\note##1{\parfillskip=0pt\hfill##1}
\leftskip=2pc\relax\let^^M=\lineending\relax}

\gdef\endalgorithm{\medskip\egroup}

}% end of obeylines

%% code example

\newif\ifcodeon
\newif\ifneedspace
{\obeylines\obeyspaces

\gdef\codelooker{\ifx\next\lineending\vskip-4pt%
\noindent\else%
\ifx\next\end\vskip-4pt\let^^M\ \else%
\vskip0pt\noindent\fi\fi}

\gdef\codelineending{\relax\futurelet\next\codelooker}

\gdef\codesamp{\medskip\global\codeontrue\baselineskip=12pt%
\parskip=2pt\tt\obeylines\obeyspaces\let^^M=\codelineending}
\gdef\endcodesamp{\global\codeonfalse\vskip12pt}

\gdef\spcodesamp{\baselineskip=12pt%
\parskip=2pt\tt\obeylines\obeyspaces\let^^M=\codelineending}

\gdef\codebox#1{\let\go\relax%
\ifcodeon\else\global\needspacetrue\let\go\spcodesamp\fi\go%
\global\dimen1=#1\relax\global\setbox0=\vbox\bgroup}

\gdef\endcodebox{\ifneedspace\global\needspacefalse\vskip\baselineskip\fi%
\egroup%
\vtop to \ht0{\vskip-3pt\hrule\hbox to\dimen1{\vrule height\ht0%
\hfill\vrule height\ht0 depth \dp0}\hrule\vss}%
\nobreak\vskip-\ht0\rlap{\hskip6pt\vbox{\unvbox0}}}}
%% <== end algorithm


%% ==> glossary
\def\glossary{\section*{GLOSSARY}
\everypar={}
\bgroup\leftskip=1pc \parindent=-1pc}
\def\term#1{\vskip2pt{\bf #1}\hskip1em\relax}
\def\endglossary{\vskip12pt\egroup}
%% <==

%%%%%%%%%%%%%%%%%%%%%%%
%% Index ===>>>


% latex default
\ifll
\newenvironment{theindex}
               {\if@twocolumn
                  \@restonecolfalse
                \else
                  \@restonecoltrue
                \fi
                \columnseprule \z@
                \columnsep 35\p@
\multicols{2}[\chapter*{Index}]
\markboth{INDEX}{INDEX}
\addcontentsline{toc}{chapter}{\protect\cnumberline{}Index}
\footnotesize
                \parindent\z@
                \parskip\z@ \@plus .3\p@\relax
                \let\item\@idxitem}
{\endmulticols
\onecolumn
\normalsize
\thispagestyle{empty}
\pagestyle{empty}
\leftskip=0pt\vskip1sp}
\newcommand{\@idxitem}  {\par\hangindent 40\p@}
\newcommand{\subitem}   {\par\hangindent 40\p@ \hspace*{20\p@}}
\newcommand{\subsubitem}{\par\hangindent 40\p@ \hspace*{30\p@}}
\newcommand{\indexspace}{\par \vskip 10\p@ \@plus5\p@ \@minus3\p@\relax}
\else
\newenvironment{theindex}
               {\if@twocolumn
                  \@restonecolfalse
                \else
                  \@restonecoltrue
                \fi
                \columnseprule \z@
                \columnsep 35\p@
\multicols{2}[\chapter*{Index}]
\markboth{INDEX}{INDEX}
\addcontentsline{toc}{chapter}{\protect\cnumberline{}Index}
\footnotesize
 \parindent\z@
                \parskip\z@ \@plus .3\p@\relax
                \let\item\@idxitem}
{\endmulticols
\onecolumn
\normalsize
\thispagestyle{empty}
\pagestyle{empty}
\leftskip=0pt\vskip1sp}
\newcommand{\@idxitem}  {\par\hangindent 40\p@}
\newcommand{\subitem}   {\par\hangindent 40\p@ \hspace*{20\p@}}
\newcommand{\subsubitem}{\par\hangindent 40\p@ \hspace*{30\p@}}
\newcommand{\indexspace}{\par \vskip 10\p@ \@plus5\p@ \@minus3\p@\relax}
\fi
%%% <<<=== end latex default Index

%%%%%%% Index Formatting

\def\testforadd{\addtoindex}

\def\seealso#1{\hskip16pt{\it See also\ }{#1}}

\def\ltr#1{\nobreak\vskip5pt
\hrule
\nobreak
\vskip3pt
{\bf#1}
\nobreak
\vskip5pt}

\let\saveangle>

\newif\ifnosrtfile
\def\newinput#1{\openin1 #1 \ifeof1 \typeout{^^J^^JWarning:
There is no file #1.^^J^^J\space\space \string\kluwerprintindex\space
will not work without a \jobname.srt file.^^J^^J
\space\space Please run your sorting program 
on \jobname.inx\space
to produce \jobname.srt.^^J^^J}\global\nosrtfiletrue\else
\global\nosrtfilefalse\closein1 \relax\input #1\fi}

\def\authorinput#1{\openin1 #1 \ifeof1 \typeout{^^J^^JWarning:
There is no file #1.^^J^^J\space\space \string\authorindex\space
will not work without a \jobname.att file.^^J^^J
\space\space Please run your sorting program 
on \jobname.aut\space
to produce \jobname.att.^^J^^J}\global\nosrtfiletrue\else
\global\nosrtfilefalse\closein1 \relax\input #1\fi}


{\catcode`>=\active
\gdef\kluwerprintindex{
\let~\relax
\catcode`>=\active%
\let>\doindex
\newinput{\jobname.srt} \catcode`\*=12 \catcode`>=12
\ifnosrtfile\let\doitnow\relax\else\let\doitnow\doindexending\fi
\doitnow
}

\gdef\doindexending{
\vskip-14pt
\endmulticols
\onecolumn
\leftskip=0pt\vskip1sp}

}

\let\saveanglebracket>
\newif\if@restonecol
{\catcode`>\active
\gdef\doindex #1{\multicols{2}[\chapter*{Index}]
\markboth{INDEX}{INDEX}
\bgroup
\let>\saveanglebracket
\def~{\relax}
\footnotesize
\noindent\ \unskip
\vskip-\baselineskip
\parindent=-\indexwrap \leftskip=\indexwrap
\def\,{\ifmmode\typeout{mmode^^J^^J}\mskip\thinmuskip\else\comma\fi}
\addcontentsline{toc}{chapter}{\protect\cnumberline{}Index}
\columnsep 35pt
\hfuzz=.5in
\def\checker{#1}
\widowpenalty0
\clubpenalty0
\parskip=0pt plus.001pt
\raggedright
\ifx\checker\testforadd\let\next=\relax\else\let\next=\beginindex\fi\next #1}
}

%%%%%%  Index Entries

\newwrite\innx

\immediate\openout\innx=\jobname.inx

\newif\ifsilent
\newif\ifnotsilent

\newcount\spcount
\def\oneast{\global\spcount=1}
\def\twoast{\global\spcount=2}

\def\astsplit#1*#2#3{{\xdef\pagenumzeros{\ifnum\count0<100
\ifnum\count0<10 00\else 0\fi\fi}\if#2\relax%
\def\,{\noexpand\comma}%
\writeit{#1}%
\else\if#3\relax
\def\,{\noexpand\comma}%
\writeita{#1}\else\def\,{\noexpand\comma}%
\writeitb{#1}\fi\fi%
\ifnotsilent#1\global\notsilentfalse\fi}}

\def\inx#1{\global\notsilenttrue\inxx{#1}{}}
\def\inxx#1{\astsplit#1*\relax\relax}

\def\romannums{\csname @roman\endcsname\c@page}

\def\writeit#1{\xdef\pagenumzeros{\ifnum\count0<100
\ifnum\count0<10 00\else 0\fi\fi}%
\ifx\thepage\romannums%
\write\innx{\noexpand#1 |{%
\pagenumzeros\the\count0\string\global\string\romanontrue}}\else%
\write\innx{\noexpand#1 |{\pagenumzeros\the\count0}}\fi}

\def\writeita#1{\xdef\pagenumzeros{\ifnum\count0<100
\ifnum\count0<10 00\else 0\fi\fi}%
\ifx\thepage\romannums%
\write\innx{\noexpand#1 |{\pagenumzeros\the\count0
\oneast\string\global\string\romanontrue}}\else
\write\innx{\noexpand#1 |{\pagenumzeros\the\count0
\oneast}}\fi}

\def\writeitb#1{\xdef\pagenumzeros{\ifnum\count0<100
\ifnum\count0<10 00\else 0\fi\fi}%
\ifx\thepage\romannums%
\write\innx{\noexpand#1 |{\pagenumzeros\the\count0
\twoast\string\global\string\romanontrue}}\else
\write\innx{\noexpand#1 |{\pagenumzeros\the\count0
\twoast}}\fi}

\write\innx{\noexpand>}
\write\innx{\noexpand~}

%% end sending to index
%%%%%% Author Index Entries

\newwrite\annx

\def\authorastsplit#1*#2#3{{\xdef\pagenumzeros{\ifnum\count0<100
\ifnum\count0<10 00\else 0\fi\fi}\if#2\relax%
\def\,{\noexpand\comma}%
\awriteit{#1}%
\else\if#3\relax
\def\,{\noexpand\comma}%
\awriteita{#1}\else
\def\,{\noexpand\comma}%
\awriteitb{#1}\fi\fi%
\ifnotsilent#1\global\notsilentfalse\fi}}

\def\authorpickup#1{\authorastsplit#1*\relax\relax}

\newif\ifauthorind
\def\anx#1{\ifauthorind\else\global\authorindtrue\startauthorindex\fi%
\global\notsilenttrue\authorpickup{#1}}
\def\anxx#1{\ifauthorind\else\global\authorindtrue\startauthorindex\fi%
\authorastsplit#1*\relax\relax}

\def\awriteit#1{\xdef\pagenumzeros{\ifnum\count0<100
\ifnum\count0<10 00\else 0\fi\fi}%
\ifx\thepage\romannums%
\write\annx{\noexpand#1 |{%
\pagenumzeros\the\count0\string\global\string\romanontrue}}\else
\write\annx{\noexpand#1 |{\pagenumzeros\the\count0}}\fi}

\def\awriteita#1{\xdef\pagenumzeros{\ifnum\count0<100
\ifnum\count0<10 00\else 0\fi\fi}%
\ifx\thepage\romannums%
\write\annx{\noexpand#1 |{\pagenumzeros\the\count0
\oneast\string\global\string\romanontrue}}\else
\write\annx{\noexpand#1 |{\pagenumzeros\the\count0
\oneast}}\fi}

\def\awriteitb#1{\xdef\pagenumzeros{\ifnum\count0<100
\ifnum\count0<10 00\else 0\fi\fi}%
\ifx\thepage\romannums%
\write\annx{\noexpand#1 |{\pagenumzeros\the\count0
\twoast\string\global\string\romanontrue}}\else
\write\annx{\noexpand#1 |{\pagenumzeros\the\count0
\twoast}}\fi}

\let\savecomma\,
\def\startauthorindex{\global\let\catchcomma\,
\gdef\,{\ifmmode\catchcomma\else\savecomma\fi}
\immediate\openout\annx=\jobname.aut
\write\annx{\noexpand>}
\write\annx{\noexpand~}}

{\catcode`>=\active
\gdef\printauthorindex{
\catcode`>=\active%
\let>\doauthorindex
\let~\relax
\authorinput{\jobname.att} \catcode`\*=12 \catcode`>=12
\ifnosrtfile\let\doitnow\relax\else\let\doitnow\doindexending\fi
\doitnow
}}

\def\doauthorindex #1{\multicols{2}[\chapter*{Author Index}]
\markboth{AUTHOR INDEX}{AUTHOR INDEX}
\bgroup
\footnotesize
\noindent\ \unskip
\vskip-\baselineskip
\parindent=-\indexwrap \leftskip=\indexwrap
\def\,{\ifmmode\typeout{mmode^^J^^J}\mskip\thinmuskip\else\comma\fi}
\addcontentsline{toc}{chapter}{\protect\cnumberline{}Author Index}
\columnsep 35pt
\hfuzz=.5in
\def\checker{#1}
\widowpenalty0
\clubpenalty0
\parskip=0pt plus.001pt
\raggedright
\ifx\checker\testforadd\let\next=\relax\else\let\next=\beginindex\fi\next #1}

{\catcode`>=\active
\gdef\printtopicindex{
\catcode`>=\active%
\let>\dotopicindex
\newinput{\jobname.srt} \catcode`\*=12 \catcode`>=12
\ifnosrtfile\let\doitnow\relax\else\let\doitnow\doindexending\fi
\doitnow
}}

{\catcode`>=\active%
\newif\if@restonecol
\gdef\dotopicindex #1{\multicols{2}[\chapter*{Topic Index}]
\markboth{TOPIC INDEX}{TOPIC INDEX}
\bgroup
\let>\saveanglebracket
\def~{\relax}
\footnotesize
\noindent\ \unskip
\vskip-\baselineskip
\parindent=-\indexwrap \leftskip=\indexwrap
\def\,{\ifmmode\typeout{mmode^^J^^J}\mskip\thinmuskip\else\comma\fi}
 \addcontentsline{toc}{apchapter}{\protect\numberline{Topic Index\string\hss}}
\columnsep 35pt
\hfuzz=.5in
\def\checker{#1}
\widowpenalty0
\clubpenalty0
\parskip=0pt plus.001pt
\raggedright
\ifx\checker\testforadd\let\next=\relax\else\let\next=\beginindex\fi\next #1}
}

%% end sending to author index

%% start formatting index

%%% Index Macros
%%% Copyright 1992, Amy Hendrickson
%%% TeXnology, Inc.
%%% All rights reserved.


\def\indexit#1{{\it\expandafter\capthis #1}}
\def\indexbf#1{{\bf\expandafter\capthis #1}}
\def\indexsl#1{{\sl\expandafter\capthis #1}}
\def\indextt#1{{\tt\expandafter\capthis #1}}

\def\lcindexit#1{{\it #1}}
\def\lcindexbf#1{{\bf #1}}
\def\lcindexsl#1{{\sl #1}}
\def\lcindextt#1{{\tt #1}}


\def\module{}\def\rightmodule{}
\def\splittocstuff#1{\global\indxnum=#1}

\def\onward{}

\def\addtoindex{\par}
\def\xaddtoindex{\addtoindex}

\def\endadd #1{\def\lfe{#1}\ifx\lfe\xaddtoindex 
\let\lfe\relax\let\more\relax\else
\if\lfe~
\let\more=\relax\let\lfe=\endindx\else
\ifx\lfe\munge\let\more=\relax\else
\ifx\lfe\testforadd\vskip1pt\let\more\relax\else
\par\let\more=\beginindex\fi\fi\fi\fi
\expandafter\more\lfe}

\newcount\indxnum
\newcount\oldindxnum

\def\comma{,\ }
\def\go{}
\def\compare{}
\def\eatthree#1,#2,{}
\def\eattwo#1,{}
\def\oldthreecol{}
\def\oldtwocol{}
\def\oldonecol{}

\newbox\savenumbox

\def\eatcommas{\ifx\twocol\empty%
\let\eat=\relax
\else
   \ifx\threecol\empty\let\eat=\eattwo\else
      \let\eat=\eatthree
    \fi
\fi\eat}

\def\splitoff#1,#2,#3,{\gdef\onecol{#1}\gdef\twocol{#2}%
\gdef\threecol{#3}\eatcommas}
\def\endin{}
\def\testind{\endin}
\def\testadd{\addtoindex}
\def\oldcompare{}
\newif\ifendindex
\newif\ifsavenum
\newcount\saveindxnum
\newif\ifnnum
\newdimen\indexwrap
\newdimen\indexindent

\def\endindx{\vskip1sp\egroup
\@normalsize}

\newif\ifromanon

\def\beginindex #1 |#2 #3{%
\parindent=-\indexwrap \leftskip=\indexwrap%
\def\module{}\let\oldrightmodule=\rightmodule%
\def\rightmodule{}%
\setbox0=\hbox{\expandafter\splittocstuff{#2}}%
\global\oldindxnum=\saveindxnum\relax%
\def\changenums{\ifnum\spcount<1%
\ifsavenum\global\savenumfalse\hbox{\unhbox\savenumbox}\fi%
, {\module\ifromanon\expandafter\romannumeral\fi%
\number\indxnum\rightmodule}\else%
\ifnum\spcount=1% 
\ifsavenum\global\savenumfalse\hbox{\unhbox\savenumbox}\fi%
, \underbar{\module\ifromanon\expandafter\romannumeral\fi%
\number\indxnum\rightmodule}%
\relax\global\spcount=0%
\else\ifnum\spcount=2%
\ifsavenum\global\savenumfalse\hbox{\unhbox\savenumbox}\relax%
\global\spcount=0\fi%
\global\spcount=0, {\bf\module\ifromanon\expandafter\romannumeral\fi%
\number\indxnum\rightmodule}%
\fi\fi\fi}%
\def\comparenums{\ifnum\oldindxnum=0\changenums\else%%%%
\ifnum\indxnum=\oldindxnum%
\ifx\oldrightmodule\rightmodule\relax%
\else%
, {\module\ifromanon\expandafter\romannumeral\fi%
\number\indxnum\rightmodule}\fi%
\else%
\advance\oldindxnum by 1\relax%
\ifnum\indxnum=\oldindxnum%
\ifnum\spcount=1%
\gdef\numinbox{\underbar{\ifromanon\expandafter\romannumeral\fi%
\number\indxnum\rightmodule}}%
\global\nnumtrue%
\else%
\ifnum\spcount=2\gdef\numinbox{\bf\ifromanon\expandafter\romannumeral\fi%
\number\indxnum\rightmodule}%
\global\nnumtrue%
\else%
\gdef\numinbox{\ifromanon\expandafter\romannumeral\fi%
\number\indxnum\rightmodule}\fi\fi%
\setbox\savenumbox=\hbox{--\hskip1sp\numinbox}%
\global\spcount=0%
\global\savenumtrue%
    \else%
\ifsavenum\global\savenumfalse\hbox{\unhbox\savenumbox}\changenums%
\else%
\changenums\fi\fi\fi\fi}%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
\def\compare{#1}% now compare words
\ifx\compare\oldcompare\comparenums% 
   %% above test seems always to be false
\else\splitoff#1,{},{},%
   \ifx\onecol\oldonecol%
      \ifx\twocol\empty%
          \ifsavenum\global\savenumfalse\hbox{\unhbox\savenumbox}%
          \fi%
            \par\global\oldindxnum=0 
            \expandafter\capthis\onecol\comparenums%
              \gdef\lastonecol{\onecol}%
      \else%% 
          \ifx\twocol\oldtwocol%
               \ifx\threecol\empty%
               \else%
                    \ifx\threecol\oldthreecol%
                    \else%
                         \ifsavenum\global\savenumfalse%
                          \hbox{\unhbox\savenumbox}%
                         \fi%
                                  \par\global\oldindxnum=0\relax%
                                 \hskip2\indexindent\threecol\comparenums%% ***
                    \fi%
               \fi%
          \else%
               \ifsavenum\global\savenumfalse\hbox{\unhbox\savenumbox}%
               \fi%
                       \global\oldindxnum=0\relax
                   \ifx\threecol\empty% 
                     \par\hskip\indexindent\twocol\comparenums%% ***
                     \gdef\lasttwocol{\twocol}%
                   \else%% this is just for error control, 
                            %% when author has new third level
                            %% but neglects second level entry.
                      \ifx\onecol\lastonecol%
                      \else\par%
                       \expandafter\capthis\onecol\gdef\lastonecol{\onecol}%
                      \fi%
                           \ifx\twocol\lasttwocol%
                           \else%
                              \par\hskip\indexindent\twocol%
                              \gdef\lasttwocol{\twocol}%
                           \fi%
                             \par\hskip2\indexindent\threecol\comparenums% ***
                   \fi%
         \fi%
     \fi%
   \else
        \ifx\twocol\empty%
            \ifsavenum\global\savenumfalse\hbox{\unhbox\savenumbox}%
            \fi% 
                \par\global\oldindxnum=0\relax%
                \expandafter\capthis\onecol\comparenums%
                \gdef\lastonecol{\onecol}%
        \else%
            \ifx\threecol\empty%
                 \ifsavenum\global\savenumfalse\hbox{\unhbox\savenumbox}%
                 \fi%
                          \global\oldindxnum=0\relax%
                    \ifx\onecol\lastonecol%
                    \else\par%
                     \expandafter\capthis\onecol\gdef\lastonecol{\onecol}%
                    \fi%
                    \par\hskip\indexindent\twocol\comparenums%% ***
            \else%
                  \ifsavenum\global\savenumfalse\hbox{\unhbox\savenumbox}%
                  \fi%
                       \global\oldindxnum=0\relax%
                  \ifx\onecol\lastonecol%
                  \else\par%
                      \expandafter\capthis\onecol\gdef\lastonecol{\onecol}%
                  \fi%
                     \ifx\twocol\lasttwocol
                     \else%
                           \par\hskip\indexindent\twocol%
                           \gdef\lasttwocol{\twocol}%
                     \fi%
                       \par\hskip2\indexindent\threecol\comparenums% ***
           \fi%
      \fi%
   \fi%
\fi%
%%%%%
\gdef\oldcompare{#1}%
\global\saveindxnum=\indxnum%
\ifnnum\global\saveindxnum=0\global\nnumfalse\fi%
\global\let\oldonecol=\onecol\relax%
\global\let\oldtwocol=\twocol% 
\global\let\oldthreecol=\threecol% 
%
\def\turnoff{#3}\ifx\turnoff\testadd% 
\ifsavenum\global\savenumfalse\hbox{\unhbox\savenumbox}\fi%
\let\go=\relax\else\ifx\turnoff\testind\let\go=\relax%
\ifsavenum\global\savenumfalse\hbox{\unhbox\savenumbox}\fi\else%
\if\turnoff~\let\go=\endindx%
\ifsavenum\global\savenumfalse\unskip\hbox{\unhbox\savenumbox}\fi%
\else\let\go=\beginindex\fi\fi\fi\global\romanonfalse\go#3}

\let\lastonecol\relax
\let\lasttwocol\relax
\let\lastthreecol\relax
\let\oldonecol\relax
\let\oldtwocol\relax
\let\oldthreecol\relax


%% CHANGE INDEX FORMAT HERE ====>

%% If you DO NOT want the first letter of each first level index
%% entry to be capitalized, delete the % in front of the following line, 
%% and put % in front of the next line:

%\let\capthis\relax
\def\capthis#1{\uppercase{#1}}%

\indexindent=8pt %% indentation for index subentries
\indexwrap=24pt  %% indentation when term is too wide for column,
                 %% continues on following lines indented this much.




%%%%%
%% Frank Mittlebach's multicol macros, minus comments and
%% irrelevancies.

\def\multicols#1{\col@number#1\relax
  \@ifnextchar[\mult@cols{\mult@cols[]}}

\def\mult@cols[#1]{\@ifnextchar[%
  {\mult@@cols{#1}}%
  {\mult@@cols{#1}[\premulticols]}}

\def\mult@@cols#1[#2]{%
   \enough@room#2%
   #1\nobreak\par%\addvspace\multicolsep 
   \begingroup
   \prepare@multicols\ignorespaces}

\def\enough@room#1{\par \penalty\z@
   \page@free \pagegoal
   \advance \page@free -\pagetotal
   \ifdim \page@free <#1\newpage \fi}

\def\prepare@multicols{%
  \output{\global\setbox\partial@page
                 \vbox{\unvbox\@cclv}}\eject
  \vbadness9999 \hbadness5000
  \tolerance\multicoltolerance 
  \doublecol@number\col@number
  \multiply\doublecol@number\tw@
  \advance\baselineskip\multicolbaselineskip
  \advance\@colroom-\ht\partial@page
  \vsize\col@number\@colroom
  \advance\vsize\c@collectmore\baselineskip
  \hsize\columnwidth \advance\hsize\columnsep
  \advance\hsize-\col@number\columnsep
  \divide\hsize\col@number 
  \linewidth\hsize 
  \output{\multi@columnout}%
%  \multiply\count\footins\col@number
%  \multiply\skip \footins\col@number
  \reinsert@footnotes}

\def\endmulticols{\parskip=0pt
\par\penalty\z@
  \output{\balance@columns}\eject\nobreak%
\endgroup
}

\newcount\c@unbalance     \c@unbalance   = 0
\newcount\c@collectmore   \c@collectmore = 0

\newcount\col@number 
\newcount\doublecol@number
\newcount\multicoltolerance
                   \multicoltolerance = 9999
\newdimen\page@free
\newdimen\premulticols  \premulticols = 50pt
\newdimen\postmulticols \postmulticols= 50pt
\newskip\multicolsep 
      \multicolsep =0pt

\newskip\multicolbaselineskip 
                   \multicolbaselineskip=0pt
\newbox\partial@page

\def\process@cols#1#2{\count@#1\relax
     \loop #2%
     \advance\count@\tw@
     \ifnum\count@<\doublecol@number 
   \repeat}


\def\page@sofar{\unvbox\partial@page 
   \process@cols\z@{\wd\count@\hsize}%
\hbox to\textwidth{%
     \process@cols\tw@{\box\count@\hss}%
\box\z@}
}


\def\reinsert@footnotes{\ifvoid\footins\else
         \insert\footins{\unvbox\footins}\fi}

\def\multi@columnout{%
   \ifnum\outputpenalty <-\@Mi
   \speci@ls \else
   \splittopskip\topskip 
   \splitmaxdepth\maxdepth
   \dimen@\@colroom
   \divide\skip\footins\col@number
   \ifvoid\footins \else
      \advance\dimen@-\skip\footins 
      \advance\dimen@-\ht\footins   \fi
   \process@cols\tw@{\setbox\count@
            \vsplit\@cclv to\dimen@}%
   \setbox\z@\vsplit\@cclv to\dimen@
   \ifvoid\@cclv \else 
       \unvbox\@cclv
       \penalty\outputpenalty\fi     
   \setbox\@cclv\vbox{\page@sofar}%
   \@makecol\@outputpage
   \global\@colroom\@colht
   \process@deferreds
   \global\vsize\col@number\@colroom 
   \global\advance\vsize
      \c@collectmore\baselineskip
   \multiply\skip\footins\col@number\fi}

\def\speci@ls{%
   \unvbox\@cclv\reinsert@footnotes
   \gdef\@currlist{}}

\def\process@deferreds{%
   \@floatplacement 
   \begingroup
    \let\@tempb\@deferlist
    \gdef\@deferlist{}%
    \let\@elt\@scolelt
      \@tempb \endgroup}

\newif\ifshr@nking

\def\raggedcolumns{%
   \@bsphack\shr@nkingtrue\@esphack}

\def\flushcolumns{%
   \@bsphack\shr@nkingfalse\@esphack}

\newcount\loopcount

\newdimen\savedimen
\newdimen\fixdimen

\def\escapeloop{\gdef\iterate{}}

\let\saveiterate\iterate

\def\balance@columns{%
\loopcount=0
\fixdimen=0pt
   \splittopskip\topskip
   \splitmaxdepth\maxdepth
   \setbox\z@\vbox{\unvbox\@cclv}\dimen@\ht\z@
   \advance\dimen@\col@number\topskip
   \advance\dimen@-\col@number\baselineskip
   \divide\dimen@\col@number
   \advance\dimen@\c@unbalance\baselineskip
   {\vbadness\@M \loop
   {\process@cols\@ne{\global\setbox\count@
                              \box\voidb@x}}%
     \global\setbox\@ne\copy\z@
     {\process@cols\thr@@{\savedimen\ht\@ne \advance\savedimen by-\dimen@
\global\setbox\count@ \vsplit\@ne to\dimen@ 
\ifdim\savedimen>\ht\@ne % AH: TeX was forced to make box bigger than \dimen@.
\advance\savedimen by-\ht\@ne \advance\savedimen by -7.1pt 
\ifdim\savedimen>\fixdimen \global\fixdimen\savedimen\fi\fi}}%
    \ifdim\ht\@ne >\ht\thr@@
    \global\advance\dimen@\p@%.5\baselineskip
\global\advance\loopcount by1
\ifnum\loopcount=90 \escapeloop\fi
   \repeat}%
\let\iterate\saveiterate
\dimen@\ht\thr@@  
\ifdim\fixdimen>0pt \advance\dimen@ by \fixdimen\fi
   \process@cols\z@{\@tempcnta\count@ 
        \advance\@tempcnta\@ne
        \setbox\count@\vtop to\dimen@
           {\unvbox\@tempcnta
           \ifshr@nking\vfill\fi}
}%
   \global\vsize\@colroom 
   \global\advance\vsize\ht\partial@page
   \page@sofar
}

%%%%%%%%%%%%%%
% Draft

\def\draft{\def\@oddfoot{\vtop to 0pt{\vskip24pt\large\tt
D R A F T
\hfill Page \thepage\hfill 
\today, \realtime\hfill D R A F T\global\titlefalse\vss}}
\def\@evenfoot{\@oddfoot}}

\def\fix{\ifcase\oldtime 0\or0\or0\or%
0\or0\or0\or0\or0\or0\or0\fi}
\def\fixtiming{\ifcase\timing 0\or0\or0\or% 
0\or0\or0\or0\or0\or0\or0\fi}
\newcount\timing 
\newcount\hourcount
\newcount\oldtime 

\def\realtime{\timing=\time \oldtime=\time
\ifnum\timing>60 \divide\timing by 60
\hourcount=\the\timing
\multiply\timing by 60
\advance\oldtime by-\timing
\ifnum\hourcount<12 \number\hourcount:\fix\number\oldtime am\fi%
\ifnum\hourcount=12 \number\hourcount:\fix\number\oldtime pm\fi%
\ifnum\hourcount>12 \advance\hourcount by-12
\number\hourcount:\fix\number\oldtime pm\fi
\else12:\fixtiming\number\timing am\fi}

\newcommand{\today}{\ifcase\month\or
  January\or February\or March\or April\or May\or June\or
  July\or August\or September\or October\or November\or December\fi
  \space\number\day, \number\year}


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% Cropmarks
\newif\ifcropmarkson
\def\printcropmarks{\global\cropmarksontrue}

\newdimen\widthofcropmarks
\widthofcropmarks=36pc%35pc
\newdimen\raisetopcropmarks
\raisetopcropmarks=-1pc
\newdimen\raisebottomcropmarks
\raisebottomcropmarks-6pc
\newdimen\movecropmarksright
\movecropmarksright2pc


\def\topcropmarks{\ifcropmarkson
\vtop to0pt{\vss
\hbox to0pt{\hskip\movecropmarksright\hbox to0pt{\hss%
\hbox to1pc{\hrulefill\hskip3pt}\raise6pt\hbox{\vrule height 1pc}}%
\hskip\widthofcropmarks%
\hbox to0pt{%
\raise6pt\hbox{\vrule height 1pc}\hbox to1pc{\hskip3pt\hrulefill}\hss}\hss}%
\vskip\raisetopcropmarks}
\fi}

\def\bottomcropmarks{\ifcropmarkson
\vtop to0pt{\vskip-\raisebottomcropmarks
\hbox to0pt{\hskip\movecropmarksright\hbox to0pt{\hss%
\hbox to1pc{\hrulefill\hskip3pt}\lower6pt\hbox{\vrule depth 1pc height 0pt}}%
\hskip\widthofcropmarks%
\hbox to0pt{%
\lower6pt\hbox{\vrule depth 1pc height0pt}\hbox to1pc{\hskip3pt\hrulefill}\hss}\hss}%
\vss}\fi}


\ifll
%% From default LaTeX2e except for parts marked ` <=== new '
\gdef\@outputpage{\begingroup 
\let\protect\noexpand\@resetactivechars\let \-\@dischyph
\let\'\@acci \let\`\@accii \let\=\@acciii \let\\\@normalcr \let\par\@@par 
\shipout\vbox{\topcropmarks%  <=== new 
\set@typeset@protect 
\aftergroup \endgroup \aftergroup \set@typeset@protect 
\if@specialpage \global \@specialpagefalse \@nameuse {ps@\@specialstyle }\fi 
\if@twoside \ifodd \count \z@ \let \@thehead \@oddhead \let \@thefoot 
\@oddfoot \let \@themargin \oddsidemargin \else \let \@thehead \@evenhead 
\let \@thefoot \@evenfoot \let \@themargin \evensidemargin \fi \fi 
\reset@font \normalsize \baselineskip \z@skip \lineskip \z@skip 
\lineskiplimit \z@ \@begindvi \vskip \topmargin \moveright \@themargin 
\vbox {\setbox \@tempboxa \vbox to\headheight 
{\vfil \color@hbox \normalcolor \hb@xt@ \textwidth {\let \label \@gobble 
\let \index \@gobble \let \glossary \@gobble \@thehead }
\color@endbox }\dp \@tempboxa \z@ \box\@tempboxa \vskip\headsep
\box\@outputbox \baselineskip \footskip \color@hbox \normalcolor 
\hb@xt@ \textwidth {\let \label \@gobble \let \index \@gobble 
\let \glossary \@gobble \@thefoot }\color@endbox }\bottomcropmarks%  <=== new 
}%
\global\@colht\textheight \stepcounter {page}\let \firstmark \botmark}
\else
%% From default LaTeX 2.09 except for parts marked ` <=== new '
\gdef\@outputpage{\begingroup\catcode`\ =10
     \let\-\@dischyph \let\'\@acci \let\`\@accii \let\=\@acciii
    \if@specialpage
     \global\@specialpagefalse\@nameuse{ps@\@specialstyle}\fi
     \if@twoside
       \ifodd\count\z@ \let\@thehead\@oddhead \let\@thefoot\@oddfoot
            \let\@themargin\oddsidemargin
          \else \let\@thehead\@evenhead
          \let\@thefoot\@evenfoot \let\@themargin\evensidemargin
     \fi\fi
     \shipout
     \vbox{\topcropmarks%  <=== new 
\reset@font %% RmS 91/08/15
           \normalsize \baselineskip\z@ \lineskip\z@
           \let\par\@@par %% 15 Sep 87
           \vskip \topmargin \moveright\@themargin
           \vbox{\setbox\@tempboxa
                   \vbox to\headheight{\vfil \hbox to\textwidth
                                       {\let\label\@gobble \let\index\@gobble
                                        \let\glossary\@gobble %% 21 Jun 91
                                         \@thehead}}% %% 22 Feb 87
                 \dp\@tempboxa\z@
                 \box\@tempboxa
                 \vskip \headsep
                 \box\@outputbox
                 \baselineskip\footskip
                 \hbox to\textwidth{\let\label\@gobble
                           \let\index\@gobble  %% 22 Feb 87
                           \let\glossary\@gobble %% 21 Jun 91
                           \@thefoot}}\bottomcropmarks%  <=== new 
}%
\global\@colht\textheight
           \endgroup\stepcounter{page}\let\firstmark\botmark}
\fi


\newif\ifturnoffs
\def\turnoff{\global\turnoffstrue}



%% These are for compatibility with 2e.
\def\_{\leavevmode \kern .06em \vbox {\hrule width.3em}}

\def\@ldots{\mathinner {\mathchar"613A \mathchar"613A \mathchar"613A }}             
\def\ldots{\relax \ifmmode \@ldots \else \mbox {$\m@th \@ldots \,$}\fi} 


%%%% \bio macro will make hanging indentation for photo
%%%% Use:
%%%% \picturewidth=<dimen>
%%%% \picturedepth=<dimen>
%%%% \bio{<name>}{<biography...>}

\newdimen\picturewidth
\newdimen\picturedepth
\newcount\linenumber
\newcount\parlinenumber

\def\pickuppar#1\par#2\par#3\par#4\par%
{\gdef\lookatpar{#1}\gdef\lookatpartwo{#2}
\gdef\lookatparthree{#3}\gdef\lookatparfour{#4}}

\long\def\bio#1#2{\bigskip{\small \parskip=0pt \parindent=6pt
\linenumber=\picturedepth % change from dimension to counter because
                          % we want to end up with a number not a dimension.
\divide\linenumber by 65536 % go from scaled points to normal points
\divide\linenumber by 8 % baselineskip in footnotesize
%
% To use \pickuppar to pick up the first paragraph:
\setbox0=\vbox{\advance\hsize by-\picturewidth \pickuppar
\noindent{\bf #1\ }#2\par\vskip1sp\par\vskip1sp\par\vskip1sp\par}
%
% To see height of first paragraph:
\setbox0=\vbox{\advance\hsize by-\picturewidth \noindent\lookatpar\vskip1sp}
\setbox1=\vbox{\advance\hsize by-\picturewidth \noindent\lookatpartwo
\vskip1sp}
\setbox2=\vbox{\advance\hsize by-\picturewidth \noindent\lookatparthree
\vskip1sp}
\setbox3=\vbox{\advance\hsize by-\picturewidth \noindent\lookatparfour
\vskip1sp}
% If there are enough lines available to fit picture in first paragraph:
\ifdim\ht0>\picturedepth
\hangindent=\picturewidth
\hangafter=-\linenumber
\noindent{\bf #1\ }#2
\else
% Now we will use two paragraphs to do the wrapping:
\parlinenumber=\ht0 % To see how many lines will be indented in first par
\divide\parlinenumber by 65536 % go from scaled points to normal points
\divide\parlinenumber by 8 % baselineskip in footnotesize
\global\advance\linenumber by-\parlinenumber 
\everypar={\global\everypar={
\hangindent=\picturewidth \hangafter=-\linenumber
\global\everypar={\doboxone 
\ifnum\linenumber>0
\hangindent=\picturewidth \hangafter=-\linenumber\fi
\global\everypar={\ifnum\linenumber>0
\doboxtwo 
\ifnum\linenumber>0
\hangindent=\picturewidth \hangafter=-\linenumber\fi
\else
\hangindent=0pt \hangafter=0
\fi
\everypar={\global\everypar={\ifnum\linenumber>0
\doboxthree
\ifnum\linenumber>0
 \hangindent=\picturewidth \hangafter=-\linenumber\fi
\else
\hangindent=0pt \hangafter=0
\fi}}}}}}
\hangindent=\picturewidth
\hangafter=-\parlinenumber
\noindent{\bf #1\ }#2\fi
\vskip1sp}\global\everypar={}}

\def\doboxone{\parlinenumber=\ht1 
\divide\parlinenumber by 65536 % go from scaled points to normal points
\divide\parlinenumber by 8 % baselineskip in footnotesize
\global\advance\linenumber by-\parlinenumber\relax}

\def\doboxtwo{\parlinenumber=\ht2 
\divide\parlinenumber by 65536 % go from scaled points to normal points
\divide\parlinenumber by 8 % baselineskip in footnotesize
\global\advance\linenumber by-\parlinenumber\relax}

\def\doboxthree{\parlinenumber=\ht3 
\divide\parlinenumber by 65536 % go from scaled points to normal points
\divide\parlinenumber by 8 % baselineskip in footnotesize
\global\advance\linenumber by-\parlinenumber\relax}


%% How many levels of section head would you like numbered?
%% 0= no section numbers, 1= section, 2= subsection, 3= subsubsection
%%==>>
\setcounter{secnumdepth}{3}


%% How many levels of section head would you like to appear in the
%% Table of Contents?
%% 0= chapter titles, 1= section titles, 2= subsection titles, 
%% 3= subsubsection titles.
%%==>>
\setcounter{tocdepth}{3}

%% Make adjustments for old versions of LaTeX2.09 ==>>
\expandafter\ifx\csname reset@font\endcsname\relax
\global\let\reset@font\relax
%%
\gdef\quote{\small\list {}{\rightmargin \leftmargin }\item []}
\gdef\endquote{\global \advance \@listdepth \m@ne \endtrivlist}
%%
\gdef\endfigure{%
\par \vskip\z@\egroup \ifnum \@floatpenalty <\z@ \@cons \@currlist \@currbox 
\ifdim \ht \@currbox >\textheight \ht \@currbox \textheight \fi 
\ifnum \@floatpenalty < -\@Mii \penalty -\@Miv \@tempdima \prevdepth 
\vbox {} \prevdepth \@tempdima \penalty \@floatpenalty 
\else \vadjust {\penalty -\@Miv \vbox {}\penalty \@floatpenalty}%
\@esphack\fi\fi}
%%
\gdef\endtable{%
\par \vskip \z@ \egroup \ifnum \@floatpenalty <\z@ \@cons \@currlist 
\@currbox \ifdim \ht \@currbox >\textheight \ht\@currbox\textheight\fi 
\ifnum \@floatpenalty < -\@Mii \penalty -\@Miv \@tempdima \prevdepth 
\vbox {} \prevdepth \@tempdima \penalty \@floatpenalty \else 
\vadjust {\penalty -\@Miv \vbox {}\penalty \@floatpenalty }\@esphack\fi\fi}
%%
\def\enddescription{%
\global \advance \@listdepth \m@ne \endtrivlist}%
\fi

%%%%%%%%%%%%%%%
\newtheorem{theorem}{Theorem}[chapter]
\def\thetheorem{\thechapter.\the\c@theorem}

\def\nochaptheoremnumbers{\def\thetheorem{\the\c@theorem}}

\newtheorem{definition}[theorem]{Definition}

\def\@begintheorem#1#2{\trivlist
   \item[\hskip \labelsep{\bfseries{\theoremfont #1\  #2}}]\itshape}

\def\@opargbegintheorem#1#2#3{\trivlist
      \item[\hskip \labelsep{\bfseries{\theoremfont #1\ #2\ (#3)}}]\itshape}

\newtheorem{example}[theorem]{Example}
\newtheorem{lemma}[theorem]{Lemma}
\newtheorem{exercise}[theorem]{Exercise}
\newtheorem{corollary}[theorem]{Corollary}


%%%% side by side
%%%% to use: 
% \begin{figure or table} \sidebyside{}{} \end{figure or table}
%
% \begin{table} 
% \sidebyside{\caption{table caption} first table}
% {\caption{table caption} second table}
% \end{table}

% or

% \begin{figure} 
% \sidebyside{\vskip<dimen>\caption{fig caption}}
% {\vskip<dimen>\caption{fig caption}}
% \end{figure}

\long\def\sidebyside#1#2{%
\hbox to\textwidth{\vtop{\hsize=.45\textwidth%
\parindent=0pt
\centering
 
#1\vskip1sp}\hfill\vtop{\hsize=.45\textwidth%
\parindent=0pt
\centering
#2

}}}

\global\upperandlowertrue
\def\useuppercase{\global\upperandlowerfalse}
\def\upperandlowercase{\global\upperandlowertrue}

\newif\ifsprunningheads
\def\chapsectrunningheads{\global\sprunningheadstrue
\def\markboth##1##2{\lhead{##1}\rhead{##2}}}


%cc
\renewcommand{\thefigure}{\arabic{figure}}
\renewcommand{\thetable}{\Roman{table}}
\setlength\leftmargini {13pt}
%cc

\endinput

Change Log
------------
April 4, 2004
Made change to \cite macros in normallatexbib
so that they don't choke when \usepackage{babel}
is used.

March 3, 2004
Change from \input to \@input to keep LaTeX from
stopping if there isn't a .bbl file

February 24, 2004
Small change to \@lbibitem to prevent new 
page starting where it shouldn't

September 29, 2003
Change to definition of \@cite so that there
will not be comma and space at end of citations
when not needed.

August 14, 2003
More space for numbers in subsubsections in TOC

August 5, 2003
Change to section def so that xref works right
whether or not \chapsect is set to be true.

June 17, 2004
Change to \theindex so that Index appears in TOC

June 4, 2003
Change to narrow caption so that it starts at the
bottom of illustration.

May 28, 2003
Made change to labeling in section so that
chapsection will get chapter.section label,
and only section label otherwise. 

Also added a little space above table caption so
that there will be space when table appears midpage.

Change to narrowcaption so that it doesn't overwrite
text above it.

April 28, 2003
Made change to \and for multiple authors in toc to
make it print in italics.

April 14, 2003
Made it possible to include an affiliation in prefaceauthor
command.

January 10, 2003
Narrowcaption now will line up next to .eps illustration,
not below it.

August 19, 2002
Continued caption and narrow caption and narrow
continued caption now will use the [] argument
to send info to LOT or LOF

Made change to numbered citations so that they
number 1--10 if they are sequential, and if the
first number is the first number of the sequence.
[1-3, 4-6] works as does [1,2,3-10,12] etc.

May 2, 2002
Changes to make author index running head work on
last page; and to make \, work correctly even if
amsmath is used.

Dec 20, 2001
No running head for acknowledgment, don't turn off
running heads in existance.

October 23, 2001
Change to equation and eqnarray to make cross-referencing
work in SWP

September 21, 2001
Change to make period appear after section number in TOC

September 10, 2001
Made \endnote \long and took out \vtop so that it
could break over pages.

September 4, 2001
Made acknowledgements in running head turn off.

August 10, 2001
Changed mathletter to resemble lettered caption so that it
also uses a new number if `a' or `A' is given as an argument.

August 6, 2001
Added a4paper option, that will add 48pt to textheight
but not change anything else in the style.

August 4, 2001
Changed definition of chapthebibliography so that
it will send `references' to TOC every time it is
used.

August 2, 2001
Made change to lettered caption to make it advance the
figure and table counter when letter given is `a' or `A'
This makes it work when there are two sets of lettered
captions following one after the other, so that the
second set advances the counter when it sees the `a'

July 18, 2001
Changed to have chapter bibliography not change
running heads to `References' but keep the same
article or book running head as previous in the article.

May 23, 2001
Made \chaptersection command which will make
sections number with chapter.section numbers.

Default is

1. section
1.1 subsection
1.1.1 subsubsection

May 3, 2001
Equalized spacing between editors' names on title page.

May 1, 2001
Made changes to section commands to make toc have
same section numbering as section heads.

Feb 19, 2001
Made lines shorter in section definitions to
hope to prevent email transmission line break
problems.

Feb 9, 2001
Made small fix so that there is no error message after
chapter with no label.

February 7, 2001
Fixed \label bug in chapter heading caused by \uppercase

January 15, 2001
Made chapter number be included in equation number

Made \useuppercase command to make section and subsection
heads be uppercase by default, including \section* and \subsection*

November 13, 2000
Changes to TOC entries for author and topic index; changes
to index ending to keep it from turning off running heads.

October 23, 2000
Fixed small bug in normallatexbib definition of
thebibliography.

October 19, 2000
Change to allow upper and lower case letters in section heads;
change to makecaption to set continuedcaptionfalse.

August 8, 2000
Changed \def\inxx{} so that it doesn't get rid of space following it.

July 26, 2000
Changed \def\inx{} so that it doesn't make extra space when used.

Got rid of extra space in \@makecaption (  \fignumfont #1% )

May 4, 2000
Changed \theequation so that no chapter number is used.

March 28, 2000
Change to running head format

March 1, 2000
Fixed bug in l@bibitem

November 16, 1999
Changed appendices so that they use letter if they are not
the first one and if they don't have a title.

Changed sidebyside to give more space between table and
figures.

August 27, 1999
Added \and command, to make `and' appear before the
last author's name in toc.

August 20, 1999
Added \sidebyside for sidebyside captions.

July 27, 1999
Change to definition of kluwerbib so that \cite{} will work
for authors wanting to use bibtex. Not necessary for authors
that are not using bibtex.

July 1999
=========
Many changes, general upgrade

April 29, 1999
Changed definition of \thesection to get rid of . which was
showing up in xref.

April 28, 1999
Got rid of . in theenumi etc.

Made comma appear between author names in TOC, when there
are multiple authors.

Changed xdef and edef to gdef in \chapter, \xauthor and \yauthor,
and definition of \eight in @sect


March 26, 1999
Made changes to \beginindex to allow math to appear in \kluwerprintindex
without bombing.

Change to definition of \inx and \inxx to keep them from allowing
a section to break over pages.

Changed definition of \narrowcaption so that xref label works
correctly.

September 8, 1998
Added a hack to keep chap references from sending info to
toc if there are two chap references in one chapter. Only
necessary to make sample pages look better; won't hurt
anything in real book production.

Set \setcounter{tocdepth}{3}, for subsections and subsubsections
to send info to toc.

Changed `Introduction' to be upper and lower case, to match
Contents, etc.

August 30, 1998
Made \booktitle[short title]{long title} use short title for
running head.

August 26, 1998
Math sizing added to all title environments; bold math added to
section heads and all other other title environments; title page
made to resemble monograph style; page numbers on chapter title
pages centered; section numbering style changed.

August 6, 1998
Changed definition of \turnoff to make running heads still work.

July 25, 1998
Changed \def~{\relax} in doindex to avoid error message;
made \turnoff include section, figure, and table numbers
so that only the section number is used, not chapt number.

May 20, 1998
Changed width of text in listoftables and listoffigures
so that it won't bump into page number; changed styles
of both to match TOC.

April 21, 1998
Changed \doindexending to get rid of \pagestyle{empty}

Apr. 8, 1998
Made index macros Amstex compatible by changing from
\ifx\turnoff\sptilde\let\go=\endindx% 
to
\if\turnoff~\let\go=\endindx% 

Apr. 7, 1998
Added printtopicindex and printauthorindex capability,
made \footnote and \thanks work in article title.

Apr. 6, 1998
Added set of curleys to \astsplit to prevent permanent redefinition
of \, .

Feb 10, 1998
fixed everypar 
from
\everypar={\hskip 0pt plus 1fil} 
to
\everypar={\hskip 0pt plus 1fil\relax}


Dec 23, 1997
Romoved \garbage

Dec 12, 1997
Changed \textheight to 45.5pc

Dec 10, 1997
Changed \textheight to 44.5pc

Nov 25, 1997
Deleted \count\footins=0, which made footnotes malfunction when
\let\footnote\savefootnote is used.

Nov 24, 1997
Fixed bug in \chapnumberline, to avoid L being eaten.

Nov 21, 1997
Change to \email to make curly brackets appear in tt font

Also a change to \chapter to make sure \normalsize is on,
in case some author forgets to use 
\begin{acknowledgments}...\end{acknowledgments} and instead
uses \acknowledgments.


Oct 20, 1997
Bug fix in \@dottedtocline,
changed spacing in l@section and l@subsection to allow horizontal
space for section 10.2.2.

Sept 18, 1997
Took out running head info from chapter references.

August 11, 1997
Made \kluwerprintindex

July 15, 1997
Made chapbibliography heading have same size blank space
before items whether or not BibTeX is used.

Changed
\DeclareRobustCommand*{\cal}{\@fontswitch{\relax}{\mathcal}}
\DeclareRobustCommand*{\mit}{\@fontswitch{\relax}{\mathnormal}}
to
\DeclareRobustCommand{\cal}{\@fontswitch{\relax}{\mathcal}}
\DeclareRobustCommand{\mit}{\@fontswitch{\relax}{\mathnormal}}

June 27, 1997
Made multiple citations have `;' between them, ie [fred, 1997; ginger, 1998]

June 26, 1997
Made it possible for citation `foobar, 1996' to break over lines.

June 25, 1997
Let multiple editors and affiliations be used, i.e.,
\editor{First Editor} 
\edaffil{First Affiliation} 

\editor{Second Editor} 
\edaffil{Second Affiliation} 

and,

Took citation out of hbox to let it from break over lines.


May 12, 1997
Got rid of space in definition of \theequation that was
causing an extra space in equation reference.

Apr 20, 1997
Redid bibliography, to make \normallatexbib and \kluwerbib
so that none of the normal latex bib styles are disabled.

Fixed \thefigure and \thetable in appendix so that they
use appendix letter.

Mar 14, 1997
Added \notes to \chapbibliography

Mar 10, 
Added chapter number to equation number in chapter
appendix.

Mar 6, 1997
Changed equation number in appendices, to only appendix letter
and equation number.

Feb 28, 97
Added theorem env., made theorem numbering change in appendices.

Feb 20, 97
made \author[author name to toc]{author} work right.


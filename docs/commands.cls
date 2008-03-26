\newcommand{\intinf} {\int\limits^{+\infty}_{-\infty}}
\newcommand{\eps} {$\varepsilon$}
\newcommand{\vfi} {$\varphi$}
\newcommand{\real} {I{\hspace{-1.4mm}}R}
\newcommand{\avektor} {a{\hspace{-3.0mm}}{\vspace{8mm}}\rightarrow}
\newcommand{\intRn} {\int\limits_{{I{\hspace{-1.1mm}}R}^n}}
\newcommand{\intRc} {\int\limits_{{I{\hspace{-1.1mm}}R}^3}}
\newcommand{\bA} {{\bf A}}
\newcommand{\bJ} {{\bf J}}
\newcommand{\bx} {{\bf x}}
\newcommand{\by} {{\bf y}}
\newcommand{\bz} {{\bf z}}
\newcommand{\bF} {{\bf F}}
\newcommand{\bff} {{\bf f}}
\newcommand{\bgg} {{\bf g}}
\newcommand{\bG} {{\bf G}}
\newcommand{\bh} {{\bf h}}
\newcommand{\bk} {{\bf k}}
\newcommand{\bl} {{\bf l}}
\newcommand{\ik} {{\it k}}
\newcommand{\nk} {\|{\bf k}\|}
\newcommand{\bxi} {{\mbox{\boldmath$\xi$}}}
\newcommand{\bn} {{\bf n}}
\newcommand{\bm} {{\bf m}}
\newcommand{\br} {{\bf r}}
\newcommand{\bs} {{\bf s}}
\newcommand{\bS} {{\bf S}}
\newcommand{\bv} {{\bf v}}
\newcommand{\bV} {{\bf V}}
\newcommand{\bw} {{\bf w}}
\newcommand{\bW} {{\bf W}}
\newcommand{\bu} {{\bf u}}
\newcommand{\bU} {{\bf U}}
\newcommand{\matS} {\mathcal{S}}
\newcommand{\matO} {\mathcal{O}}
\newcommand{\vecnab} {\vec{\nabla}}
\newcommand{\bvx} {\vec{\bf x}}
\newcommand{\bvy} {\vec{\bf y}}
\newcommand{\bvk} {\vec{\bf k}}
\newcommand{\etab} {{\bf \eta}}
\newcommand{\duzd} {{\mbox{d}}}
\newcommand{\duzn} {{\mbox{n}}}
\newcommand{\duzx} {{\mbox{x}}}
\newcommand{\duzy} {{\mbox{y}}}
\newcommand{\duzz} {{\mbox{z}}}
\newcommand{\duzS} {{\mbox{S}}}
\newcommand{\xxi} {x_{\xi}}
\newcommand{\xeta} {x_{\eta}}
\newcommand{\xzeta} {x_{\zeta}}
\newcommand{\yxi} {y_{\xi}}
\newcommand{\yeta} {y_{\eta}}
\newcommand{\yzeta} {y_{\zeta}}
\newcommand{\zxi} {z_{\xi}}
\newcommand{\zeeta} {z_{\eta}}
\newcommand{\zzeta} {z_{\zeta}}
\newcommand{\ve} {\vec{e}}
\newcommand{\vxxi} {\vec{x}_{\xi}}
\newcommand{\vxeta} {\vec{x}_{\eta}}
\newcommand{\vxzeta} {\vec{x}_{\zeta}}
\newcommand{\vyxi} {\vec{y}_{\xi}}
\newcommand{\vyeta} {\vec{y}_{\eta}}
\newcommand{\vyzeta} {\vec{y}_{\zeta}}
\newcommand{\vzxi} {\vec{z}_{\xi}}
\newcommand{\vzeta} {\vec{z}_{\eta}}
\newcommand{\vzzeta} {\vec{z}_{\zeta}}
\newcommand{\pp} [2] {\partial#1 \over \partial#2}
\newcommand{\ppy} [1] {\partial \over \partial#1}
\newcommand{\ppi} [2] {\partial^2#1 \over \partial#2^2}
\newcommand{\ppu} [3] {\partial^#1#2 \over \partial#3^#1}
\newcommand{\ppm} [3] {\partial^2#1 \over \partial#2\partial#3}
\newcommand{\ppd} [2] {\partial#1 / \partial#2}
\newcommand{\ppdu} [3] {\partial^#1#2 / \partial#3^#1}
\newcommand{\ppdm} [3] {\partial^2#1 / \partial#2\partial#3}
\newcommand{\DD} [2] {D#1 \over D#2}
\newcommand{\dd} [2] {d #1 \over d #2}
\newcommand{\ddu} [3] {d^{#1} #2 \over d #3^{#1}}
\newcommand{\Red}[1]{\textcolor[named]{Red}{#1}}
\newcommand{\Green}[1]{\textcolor[named]{Green}{#1}}
\newcommand{\Blue}[1]{\textcolor[named]{Blue}{#1}}
\newcommand{\boxed}[1]{\fbox{$ \displaystyle #1 $}}
\newcommand{\nin} {\in\hspace{-3mm}/}
\newcommand{\no}{\noindent}
\newcommand{\yes}{\indent}
\newcommand{\me}[3]{#1 \ddot{x} + #2\dot{x} + #3x}
\newcommand{\waveeqn} [2] {{1 \over #1^2}{\ppu{2}{#2}{t}}-\nabla^2{#2}}
\newcommand{\cwaveeqn} [2] {{\ppu{2}{#2}{t}}-#1^2\nabla^2{#2}}
\newcommand{\owaveeqn} [3] {{1 \over #1^2}{\ppu{2}{#2}{t}}-{\ppu{2}{#2}{#3}}}
\newcommand{\ocwaveeqn} [3] {{\ppu{2}{#2}{t}}-#1^2{\ppu{2}{#2}{#3}}}
\newcommand{\helm} [1] {\left(\nabla^2 + #1^2 \right)}
\newcommand{\ssum} [2] {\sum\limits_{#1}^{#2}}


% symbol fonts ...
% to be used with .. \usepackage{amsbsy}
\DeclareSymbolFont{AMSb}{U}{msb}{m}{n}
\DeclareMathSymbol{\N}{\mathbin}{AMSb}{"4E}
\DeclareMathSymbol{\Z}{\mathbin}{AMSb}{"5A}
\DeclareMathSymbol{\R}{\mathbin}{AMSb}{"52}
\DeclareMathSymbol{\Q}{\mathbin}{AMSb}{"51}
\DeclareMathSymbol{\I}{\mathbin}{AMSb}{"49}
\DeclareMathSymbol{\C}{\mathbin}{AMSb}{"43}

%----------------------------------------------------------------
%    commando \alpheqn voegt letters aan het formule-nummer
%    toe, (bijvoorbeeld 1.35a, 1.35b) en houdt het nummer gelijk,
%    tot het commando \reseteqn aangeroepen wordt

%\newcounter{saveeqn}%
%\newcommand{\alpheqn}{\setcounter{saveeqn}{\value{equation}}%
%\stepcounter{saveeqn}\setcounter{equation}{0}%
%\renewcommand{\theequation}
%    {\mbox{\arabic{chapter}.\arabic{saveeqn}-\alph{equation}}}}%
%\newcommand{\reseteqn}{\setcounter{equation}{\value{saveeqn}}%
%\renewcommand{\theequation}{\arabic{chapter}.\arabic{equation}}}%
%%%%%%%
% List with a diamond
\newcommand{\li}[1]{\item{#1}}
\newcommand{\bdli}[0]{
                     \begin{list}{$\diamond$}
                     {\setlength{\parsep}{0.0ex plus2.0ex}
                      \setlength{\topsep}{0.0ex plus2.0ex}} 
                      \setlength{\itemsep}{0.0ex plus2.0ex}}
\newcommand{\edli}[0]{\end{list}\vspace{0.3cm}}
% List with a star
\newcommand{\bsli}[0]{
                     \begin{list}{$\star$}
                     {\setlength{\parsep}{0.0ex plus2.0ex}
                      \setlength{\topsep}{0.0ex plus2.0ex}} 
                      \setlength{\itemsep}{0.0ex plus2.0ex}}
\newcommand{\esli}[0]{\end{list}\vspace{0.3cm}}
% List with a bullet
\newcommand{\bbli}[0]{
                     \begin{list}{$\bullet$}
                     {\setlength{\parsep}{0.0ex plus2.0ex}
                      \setlength{\topsep}{0.0ex plus2.0ex}} 
                      \setlength{\itemsep}{0.0ex plus2.0ex}}
\newcommand{\ebli}[0]{\end{list}\vspace{0.3cm}}
%
% List with a number  
\newcounter{nlic}
\newcommand{\bnli}[0]{
                     \begin{list}{\arabic{nlic}.}
                     {\usecounter{nlic}
                      \setlength{\parsep}{0.0ex plus0.0ex}
                      \setlength{\topsep}{0.0ex plus0.0ex}}
                      \setlength{\itemsep}{0.0ex plus0.0ex}}
\newcommand{\enli}[0]{\end{list}\vspace{0.3cm}}
% List with  circle
\newcommand{\bcli}[0]{
                     \begin{list}{$\circ$}
                     {\setlength{\parsep}{0.0ex plus0.0ex}
                      \setlength{\topsep}{0.0ex plus0.0ex}}
                      \setlength{\itemsep}{0.0ex plus0.0ex}}
\newcommand{\ecli}[0]{\end{list}\vspace{0.3cm}}
%
% List with  no symbol
\newcommand{\bli}[0]{
                     \begin{list}{}
                     {\setlength{\parsep}{0.0ex plus0.0ex}
                      \setlength{\topsep}{0.0ex plus0.0ex}}
                      \setlength{\itemsep}{0.0ex plus0.0ex}}
\newcommand{\eli}[0]{\end{list}\vspace{0.3cm}}
%
%
\newcommand{\beqn}{\begin{equation}}
\newcommand{\eeqn}{\end{equation}}
\newcommand{\beqnn}{\begin{eqnarray*}}
\newcommand{\eeqnn}{\end{eqnarray*}}
\newcommand{\beqna}{\begin{eqnarray}}
\newcommand{\eeqna}{\end{eqnarray}}
%
%
%
% newbox commands to use with picture environment
%
\newsavebox{\cube}
\savebox{\cube}{\begin{picture}(0,0)
\setlength{\unitlength}{1cm}
\put(0.0,0.0){\line(1,0){3}}
\put(3.0,0.0){\line(0,1){3}}
\put(3.0,3.0){\line(-1,0){3}}
\put(0.0,3.0){\line(0,-1){3}}
\put(0.0,3.0){\line(1,1){1.5}}
\put(1.5,4.5){\line(1,0){3}}
\put(4.5,4.5){\line(-1,-1){1.5}}
\put(4.5,4.5){\line(0,-1){3}}
\put(4.5,1.5){\line(-1,-1){1.5}}
{\linethickness{0.3mm}
\put(1.5,1.5){\qbezier[15](0.0,0.0)(0.0,1.5)(0.0,3.0)}
\put(0.0,0.0){\qbezier[15](0.0,0.0)(0.75,0.75)(1.5,1.5)}
\put(1.5,1.5){\qbezier[15](0.0,0.0)(1.5,0.0)(3.0,0.0)}}
\end{picture}}
\newsavebox{\coordxi}
\savebox{\coordxi}{\begin{picture}(0,0)
\setlength{\unitlength}{1cm}
\put(0,0.0){\vector(1,0){1}}
\put(0,0.0){\vector(0,1){1}}
\put(0,0.0){\vector(1,1){0.5}}
\put(1.2,0.0){$\xi$}
\put(-0.3,1.0){$\zeta$}
\put(0.7,0.7){$\eta$}
\end{picture}}
\newsavebox{\coordxyz}
\savebox{\coordxyz}{\begin{picture}(0,0)
\setlength{\unitlength}{1cm}
\put(0,0.0){\vector(1,0){1}}
\put(0,0.0){\vector(0,1){1}}
\put(0,0.0){\vector(1,1){0.5}}
\put(1.2,0.0){$x$}
\put(-0.3,1.0){$z$}
\put(0.7,0.7){$y$}
\end{picture}}

%
%
%


%%% Windows Turkish True Type Font -> TeX accents (Turkish) conversion
%%% for use in Win Edit and similar windows editors (wordpad, notepad, word etc)
%%%%%%%
\catcode240=13 \def ð{\u g}
\catcode231=13 \def ç{\c c}
\catcode246=13 \def ö{\"o}
\catcode254=13 \def þ{\c s}
\catcode252=13 \def ü{\"u}
\catcode253=13 \def ý{{\i }}
\catcode221=13 \def Ý{\.I}
\catcode199=13 \def Ç{\c C}
\catcode208=13 \def Ð{\u G}
\catcode214=13 \def Ö{\"O}
\catcode222=13 \def Þ{\c S}
\catcode220=13 \def Ü{\"U}


% New commands
%
\newcommand\etal{\mbox{\textit{et al.}}}
\newcommand\etc{etc.\ }
\newcommand\eg{e.g.\ }

\newtheorem{lemma}{Lemma}
\newtheorem{corollary}{Corollary}
%
% New figure commands
%
\newlength{\figw}
\newlength{\figh}
\setlength{\figw}{.90\textwidth}
\setlength{\figh}{.7\figw}

\newcommand{\Fig}[1]{figure~\ref{#1}}

\newcommand{\Figref}[1]
{figure~\ref{#1}}

%
% New mathematical commands
%
%\newcommand{\eqsize}{\normalsize}
\newcommand{\eqsize}{\small}
\newcommand{\tabsize}{\small}

\newcommand{\deriv}[2]
{\frac{\partial #1}{\partial #2}}

\newcommand{\half}{\frac{1}{2}}

\newcommand{\brk}[3]
{\frac{#1#2\alpha}{#3}}

\newcommand{\usr}{u_r}
\newcommand{\uhsr}{\hat{u}_r}
\newcommand{\utsr}{\tilde{u}_r}
\newcommand{\ust}{u_{\theta}}
\newcommand{\uhst}{\hat{u}_{\theta}}
\newcommand{\utst}{\tilde{u}_{\theta}}
\newcommand{\usz}{u_z}
\newcommand{\uhsz}{\hat{u}_z}
\newcommand{\utsz}{\tilde{u}_z}

\newcommand{\ddr}[1]
{\frac{\partial #1}{\partial r}}

\newcommand{\deedr}[1]
{\frac{d #1}{d r}}

\newcommand{\deetwodr}[1]
{\frac{d^2 #1}{d r^2}}

\newcommand{\dtwodr}[1]
{\frac{\partial^2 #1}{\partial r^2}}

\newcommand{\dtwodrdtheta}[1]
{\frac{\partial^2 #1}{\partial r\partial \theta}}

\newcommand{\dthreedr}[1]
{\frac{\partial^3 #1}{\partial r^3}}

\newcommand{\ddtheta}[1]
{\frac{\partial #1}{\partial \theta}}

\newcommand{\dtwodtheta}[1]
{\frac{\partial^2 #1}{\partial \theta^2}}

\newcommand{\dthreedtheta}[1]
{\frac{\partial^3 #1}{\partial \theta^3}}

\newcommand{\ddz}[1]
{\frac{\partial #1}{\partial z}}

\newcommand{\dtwodz}[1]
{\frac{\partial^2 #1}{\partial z^2}}

\newcommand{\Tayfirst}[3]
{\left(\frac{\partial #1}{\partial #2}\right)_{#3}}

\newcommand{\Taysecond}[3]
{\left(\frac{\partial^2 #1}{\partial #2^2}\right)_{#3}}

\newcommand{\Tayboth}[3]
{\left(\frac{\partial^2 #1}{\partial #2^2}+\frac{\partial^2 #1}{\partial #3^2}\right)}

\newcommand{\Taythird}[3]
{\left(\frac{\partial^3 #1}{\partial #2^3}\right)_{#3}}

\newcommand{\lapk}
{\Big\{\deriv{^2}{x^2} + \deriv{^2}{y^2} - k^2\Big\}}

%
% toegevoegd door F.Put
\newcommand{\dr}[2]{{\frac{d #1}{d #2}}}
\newcommand{\sint} {\int\limits}
\newcommand{\hlf} {\textstyle \frac {1} {2} \displaystyle }
\newcommand{\onethird} {\textstyle \frac {1} {3} \displaystyle}
\newcommand{\twothird} {\textstyle \frac {2} {3} \displaystyle}
\newcommand{\fourthird} {\textstyle \frac {4} {3} \displaystyle}
\newcommand{\onefourth} {\textstyle \frac {1} {4} \displaystyle}
\newcommand{\be} {\begin{equation}}
\newcommand{\ee} {\end{equation}}
\newcommand{\ebf}[1] {\mbox{\bf #1}}
\newcommand{\Kn}  {K \! n}
\newcommand{\Nu}  {N \! u}
\newcommand{\XG}  {X \! G}
\renewcommand{\Pr}  {P \! r}
\newcommand{\Sc}  {S \! c}
\newcommand{\sh}  {\!\!\!&=&\!\!\!}

% einde toevoeging
%
% Carl's toevoegingen
%
\newcommand{\mbf}[1]{\boldsymbol{#1}}
\newcommand{\re}[0] {\mbox{Re}} 
\newcommand{\im}[0] {\mbox{Im}}
\newcommand{\Hz} {H\hspace{-0.8mm}z}
\newcommand{\kHz} {\hspace{0.6mm}k\hspace{-0.6mm}H\hspace{-0.8mm}z}
\newcommand{\MHz} {\hspace{0.6mm}M\hspace{-0.6mm}H\hspace{-0.8mm}z}
\newcommand{\GHz} {\hspace{0.6mm}G\hspace{-0.6mm}H\hspace{-0.8mm}z}
\newcommand{\dB} {d\hspace{-0.6mm}B}
\newcommand{\Pa} {P\hspace{-0.8mm}a}
\DeclareSymbolFont{AMSb}{U}{msb}{m}{n}
\DeclareMathSymbol{\N}{\mathbin}{AMSb}{"4E}
\DeclareMathSymbol{\Z}{\mathbin}{AMSb}{"5A}
\DeclareMathSymbol{\R}{\mathbin}{AMSb}{"52}
\DeclareMathSymbol{\Q}{\mathbin}{AMSb}{"51}
\DeclareMathSymbol{\I}{\mathbin}{AMSb}{"49}
\DeclareMathSymbol{\C}{\mathbin}{AMSb}{"43}

\newcommand{\Res} [2] {\begin{array}{c}\raisebox{-2ex}{\mbox{Res}} \\
        \raisebox{1ex}{{\tiny $#1=#2$}}\end{array}}
%
% Einde
%
%
% ########################################################
% 
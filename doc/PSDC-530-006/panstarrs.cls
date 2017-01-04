%%
%% This is file `panstarrs.cls'
%%
%% Derived from milstd498.cls by William DeMeo @ MHPCC
%% Eugene Magnier, 2003.12.15
%%
%% Example usage:
%%
%% \documentclass[panstarrs]{panstarrs}

\NeedsTeXFormat{LaTeX2e}
\ProvidesClass{panstarrs}[2003/06/29 LaTeX class for PanSTARRS]
\LoadClass[11pt]{article}
\RequirePackage{tocloft,ifthen,fancyhdr,vmargin,longtable}
\RequirePackage[plainpages=false]{hyperref} %avoid ``destination with the same identifier'' errors
\RequirePackage{times,amsmath,amssymb,multirow}
\RequirePackage{color,fullpage,boxedminipage,acronym,epsfig}
\RequirePackage{psmisc}

% -- panstarrs definitions--
\DeclareOption{panstarrs}{\input{panstarrs.def}}
\DeclareOption{spec}{\input{spec.def}}

% -- page layout options --
\addtolength{\parskip}{+0.1in}
\setlength{\parindent}{0in}
\hoffset =-0.5in
\topmargin=0in
\oddsidemargin=0in
\evensidemargin=0in
\voffset=-0.5in
\textheight=9.0in
\textwidth=7.35in
\headheight=15pt
\headsep = 24pt
\footskip=30pt

\font\fontA=phvr at 18pt
\font\fontB=phvr at 12pt

\addtolength{\cftfignumwidth}{2em}

%  % -- SRS section numbering / plain enumerate types --
%  \let\@section=\section 
%  \let\@subsection=\subsection
%  \let\@subsubsection=\subsubsection
%  
%  \def\section{
%  %  uncomment these to label figures by section number
%  %  \renewcommand{\thefigure}{\thesection -\arabic{figure}} 
%  %  \setcounter{figure}{0} 
%    \@section
%  }
%  
%  \def\subsection{
%  %  uncomment these to label figures by section number
%  %  \renewcommand{\thefigure}{\thesubsection -\arabic{figure}} 
%  %  \setcounter{figure}{0} 
%    \@subsection
%  }
%  
%  \def\subsubsection{
%  %  uncomment these to label figures by section number
%  %  \renewcommand{\thefigure}{\thesubsubsection -\arabic{figure}} 
%  %  \setcounter{figure}{0} 
%    \@subsubsection
%  }
%  
%  \def\paragraph{
%    \@startsection{paragraph}{4}{\z@}%
%      {-3.5ex\@plus -1ex \@minus -.2ex}%
%      {1.5ex \@plus .2ex}%
%      {\normalsize\bf}
%  }
%  
%  \def\subparagraph{
%    \@startsection{subparagraph}{5}{\z@}%
%      {-3.5ex\@plus -1ex \@minus -.2ex}%
%      {1.5ex \@plus .2ex}%
%      {\normalsize\bf}
%  }
%  
%  \def\subsubparagraph{
%    \@startsection{subsubparagraph}{6}{\z@}%
%      {-3.5ex\@plus -1ex \@minus -.2ex}%
%      {1.5ex \@plus .2ex}%
%      {\normalsize\it}
%  } 
%  
%  \let\subsubsubsection=\paragraph
%  \let\subsubsubsubsection=\subparagraph


% -- section display --
\setcounter{secnumdepth}{5} % lowest level at which counters are displayed
\setcounter{tocdepth}{3} % lowest level to be included in toc

% -- basic document labels --
\newcommand\thedocnumber{PSDC-xxx-xxx} 
\newcommand\thegroup{panstarrs group}
\newcommand\theorganization{Institute for Astronomy} 
\newcommand\theshorttitle{short title} 
\newcommand\theversion{xx} 
\newcommand\theproject{panstarrs project} 
\newcommand\thesubtitle{} 
\newcommand\thedistribution{Approved for Public Release -- Distribution is Unlimited}
\newcommand\theaudience{panstarrs team} 

\newcommand{\docnumber}[1]{\renewcommand\thedocnumber{#1} }
\newcommand{\group}[1]{ \renewcommand\thegroup{#1} }
\newcommand{\organization}[1]{ \renewcommand\theorganization{#1} }
\newcommand{\shorttitle}[1]{ \renewcommand\theshorttitle{#1} }
\newcommand{\version}[1]{ \renewcommand\theversion{#1} }
\newcommand{\project}[1]{ \renewcommand\theproject{#1} }
\newcommand{\subtitle}[1]{ \renewcommand\thesubtitle{#1} }
\newcommand{\distribution}[1]{ \renewcommand\thedistribution{#1} }
\newcommand{\audience}[1]{ \renewcommand\theaudience{#1} }

%%%%%%%%%%%%%%%%%%%% start of time code %%%%%%%%%%%%%%%%%%%% 
%
% Code for printing the time by Michael Doob <mdoob@cc.umanitoba.ca>
%
\newcount\hour \newcount\minute
\hour=\time  \divide \hour by 60
\minute=\time
\loop \ifnum \minute > 59 \advance \minute by -60 \repeat
\def\nowtwelve{\ifnum \hour<13 \number\hour:% 		% supresses leading 0's
                      \ifnum \minute<10 0\fi%		% so add it it
                      \number\minute
                      \ifnum \hour<12 \ A.M.\else \ P.M.\fi
	 \else \advance \hour by -12 \number\hour:% 	% supresses leading 0's
                      \ifnum \minute<10 0\fi%		% add it in
                      \number\minute \ P.M.\fi}
\def\nowtwentyfour{\ifnum \hour<10 0\fi% 		% need a leading 0 
		\number\hour:% 				% supresses leading 0's
         	\ifnum \minute<10 0\fi% 		% add it in
         	\number\minute}
\def\now{\nowtwelve}
%%%%%%%%%%%%%%%%%%%% end of time code %%%%%%%%%%%%%%%%%%%%


% -- MAKETITLE --
\def\maketitle{
   \fancyhead{}
   \fancyhead[L]{\theproject}
   \fancyhead[R]{\thedocnumber-\theversion}
   \fancyfoot[L]{\theshorttitle}
   \fancyfoot[R]{\today}
   \par
   \begingroup
   \newpage
   \global\@topnum\z@        % Prevents figures from going at top of page.
   \@maketitle
   \endgroup
   \let\maketitle\relax
   \let\@maketitle\relax
   \gdef\@thanks{}\gdef\@author{}\gdef\@title{}\let\thanks\relax
   \pagestyle{fancy}
}

\def\@maketitle{
      \pagenumbering{roman} 
      \thispagestyle{empty} 
      \unitlength 1.0in
      \begin{picture}(0.0,0.0)(0.0,9.375)
      \small
    
      \put (7.50, 9.85){\makebox(0,0)[br]{\small      {\bf Pan-STARRS Document Control}}}
      \put (7.50, 9.70){\makebox(0,0)[br]{\small      {\bf \thedocnumber-\theversion}}}
    
      \put (4.006, 8.30){\line(1,0){0.1}} % put the macron on Manoa
      \put (0.50, 8.10){\makebox(0,0)[bl]{\fontA UNIVERSITY OF HAWAII AT MANOA}}
      \put (0.50, 7.88){\makebox(0,0)[bl]{\fontB Institute for Astrononmy}}
    
      \put (0.50, 7.83){\line(1,0){6.5}}
      \put (0.50, 7.63){\makebox(0,0)[bl]{\small      {\bf Pan-STARRS Project Management System}}}
    
      \put (3.75, 5.25){\makebox(0,0)[bc]{\large {\bf \@title}}}
      \put (3.75, 5.00){\makebox(0,0)[bc]{\large {\bf \thesubtitle}}}
    
      \put (2.15, 4.75){\makebox(0,0)[l]{{\bf Grant Award No. }}}
      \put (2.15, 4.57){\makebox(0,0)[l]{{\bf Prepared For    }}}
      \put (2.15, 4.39){\makebox(0,0)[l]{{\bf Prepared By     }}}
      \put (2.15, 4.21){\makebox(0,0)[l]{{\bf Document No.    }}}
      \put (2.15, 4.03){\makebox(0,0)[l]{{\bf Document Date   }}}
      \put (2.15, 3.85){\makebox(0,0)[l]{{\bf Revision        }}}
    
      \put (3.75, 4.75){\makebox(0,0)[l]{{\bf : F29601-02-1-0268}}}
      \put (3.75, 4.57){\makebox(0,0)[l]{{\bf : \theaudience}}}
      \put (3.75, 4.39){\makebox(0,0)[l]{{\bf : \@author}}}
      \put (3.75, 4.21){\makebox(0,0)[l]{{\bf : \thedocnumber-\theversion}}}
      \put (3.75, 4.03){\makebox(0,0)[l]{{\bf : \today}}}
      \put (3.75, 3.85){\makebox(0,0)[l]{{\bf : \theversion}}}
    
      \put (3.75, 2.50){\makebox(0,0)[c]{{\bf DISTRIBUTION STATEMENT}}}
      \put (3.75, 2.30){\makebox(0,0)[c]{{\bf \thedistribution}}}
    
      \put (3.75, 1.30){\makebox(0,0)[bc]{\footnotesize {\copyright Institute for Astronomy, University of Hawaii}}}
      \put (3.75, 1.15){\makebox(0,0)[bc]{\footnotesize {2680 Woodlawn Drive, Honolulu, Hawaii 96822}}}
      \put (3.75, 1.00){\makebox(0,0)[bc]{\footnotesize {An Equal Opportunity/Affirmative Action Institution}}}
      \end{picture}
    
      \pagebreak 
      \unitlength 1.0in
      \begin{picture}(0.0,0.0)(0.5,9.375)
      \small
    
      \put (1.00, 8.50){\makebox(0,0)[l]{Submitted By:}}
      \put (1.00, 8.00){\line(1,0){5.5}}
      \put (6.70, 8.00){\line(1,0){0.8}}
      \put (1.00, 7.90){\makebox(0,0)[l]{[Insert Signature Block of Authorized Developer Representative]}}
      \put (6.70, 7.90){\makebox(0,0)[l]{Date}}
    
      \put (1.00, 7.00){\makebox(0,0)[l]{Approved By:}}
      \put (1.00, 6.50){\line(1,0){5.5}}
      \put (6.70, 6.50){\line(1,0){0.8}}
      \put (1.00, 6.40){\makebox(0,0)[l]{[Insert Signature Block of Customer Developer Representative]}}
      \put (6.70, 6.40){\makebox(0,0)[l]{Date}}
    
      \end{picture}
      \pagebreak 
}               

% -- panstarrs definitions--
\DeclareOption{psreport}{\input{psreport.def}}

\ProcessOptions\relax

% -- some additional PanSTARRS options
% A label that also provides a hyperlink anchor
\newcommand{\hlabel}[1]{\label{#1}\relax\hypertarget{#1}{}}

% Make verbatim use a \footnotesize font (to allow 110-character lines)
\let\oldverbatim@font=\verbatim@font
\def\verbatim@font{\oldverbatim@font\scriptsize}

\def\RevisionsStart{
{ \Large \bf Revision History}
\begin{center}
\begin{tabular}{|p{1.25in}|p{1in}|p{4in}|}
\hline
{\bf Revision Number} & {\bf Release Date} & {\bf Description} \\
\hline
}
\def\RevisionsEnd{
\hline
\end{tabular}
\end{center}
\pagebreak
}

\def\TBDsStart{
{ \Large \bf TBD / TBR Listing}
\begin{center}
\begin{tabular}{|p{1in}|p{0.8in}|p{0.8in}|p{3.45in}|}
\hline
{\bf Section No.} & {\bf Page No.} & {\bf TBD/R No.} & {\bf Description} \\
\hline
}
\def\TBDsEnd{
\hline
\end{tabular}
\end{center}
\pagebreak
}

\def\DocumentsInternalSection{
\section{Referenced Documents}
\subsection{Internal Documents}
\begin{center}
\begin{tabular}{|p{1.25in}|p{5in}|}
\hline
{\bf Reference} & {\bf Title} \\
\hline}

\def\DocumentsExternalSection{
\hline
\end{tabular}
\end{center}
\par
\subsection{External Documents}
\begin{center}
\begin{tabular}{|p{1.25in}|p{5in}|}
\hline
{\bf Reference} & {\bf Title} \\
\hline}

\def\DocumentsInternal{
{\large \bf Referenced Documents}\par
\begin{center}
\begin{tabular}{|p{1.25in}|p{5in}|}
\multicolumn{1}{l}{\bf Internal Documents} \\
\hline
{\bf Reference} & {\bf Title} \\
\hline}

\def\DocumentsExternal{
\hline
\end{tabular}
\end{center}
\par
\begin{center}
\begin{tabular}{|p{1.25in}|p{5in}|}
\multicolumn{1}{l}{\bf External Documents} \\
\hline
{\bf Reference} & {\bf Title} \\
\hline}

\def\DocumentsEnd{
\hline
\end{tabular}
\end{center}
\pagebreak }

% Backwards compatibility
\let\milsection=\section
\let\milsubsection=\subsection
\let\milsubsubsection=\subsubsection
\let\milparagraph=\paragraph
\let\milsubparagraph=\subparagraph


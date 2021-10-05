#!/usr/bin/env bash

ASN="${1}"
SIGFILE="${2}"

case $ASN in
	scanner)
		FULLASN="Scanner"
		;;
	parser)
		FULLASN="Parser"
		;;
	symtable)
		FULLASN="Type Checking"
		;;
	codegen)
		FULLASN="Code Generation"
		;;
esac

if [ -z "${FULLASN}" ] || [ -z "${SIGFILE}" ]; then
	echo "usage: ${0} <scanner|parser|symtable|codegen> <path_to_sigfile>"
	exit 1
fi
if [ ! -f "${SIGFILE}" ]; then
	echo "'${SIGFILE}' could not be found or opened"
	exit 2
fi

TEXROOT="plagdecl-${ASN}"
TEXFILE="${TEXROOT}.tex"
AUXFILE="${TEXROOT}.aux"
LOGFILE="${TEXROOT}.log"

source personal.txt

cat > ${TEXFILE} <<__HERE__
\\documentclass[12pt]{article}
\\usepackage[utf8]{inputenc}
\\usepackage{graphicx}
\\usepackage{pslatex}
\\begin{document}
\\thispagestyle{empty}
\\begin{center}
\\Large\\bfseries
Plagiarism Declaration \\\\ Computer Science 244 \\\\ Stellenbosch University, 2020
\\end{center}
\\vspace{1em}\\par
\\noindent\\includegraphics[width=\\textwidth]{plagdecl.pdf}
\\vspace{1em}\\par
\\begin{tabular}{@{}rl}
Assignment: & Compiler Project (${FULLASN}) \\\\[1ex]
Full names and surname: & ${FQNAME} \\\\[1ex]
Student number: & ${SUNUM} \\\\[1ex]
Date: & $(date +"%-d %B %Y") \\\\[1ex]
Signature: & \\includegraphics[width=7em,height=7em,keepaspectratio]{${SIGFILE}}
\\end{tabular}
\\end{document}
__HERE__

pdflatex -halt-on-error ${TEXFILE} >/dev/null \
	&& pdflatex -halt-on-error ${TEXFILE} >/dev/null \
	&& (echo "Signed successfully!" ; rm ${TEXFILE} ; rm ${AUXFILE} ; rm ${LOGFILE}) \
	|| echo "Signing failed."

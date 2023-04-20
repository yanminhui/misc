#!/usr/bin/env bash

set -e

#
# cecho - Write arguments to the standard output
#
readonly COLOR_BLACK=0
readonly COLOR_RED=1
readonly COLOR_GREEN=2
readonly COLOR_YELLOW=3
readonly COLOR_BLUE=4
readonly COLOR_MAGENTA=5
readonly COLOR_CYAN=6
readonly COLOR_WHITE=7

function cecho() {
    local OPTION OPTARG OPTIND
    while getopts 'niswebuf:g:L:' OPTION; do
        case "$OPTION" in
        n)
            local DONT_APPEND_NEWLINE='-n'
            ;;
        i)
            tput bold
            tput setaf $COLOR_CYAN
            ;;
        s)
            tput bold
            tput setaf $COLOR_GREEN
            ;;
        w)
            tput bold
            tput setaf $COLOR_YELLOW
            ;;
        e)
            tput bold
            tput setaf $COLOR_RED
            ;;
        b)
            tput bold
            ;;
        u)
            tput smul
            ;;
        f)
            tput setaf $OPTARG
            ;;
        g)
            tput setab $OPTARG
            ;;
        L)
            local HYPERLINK=$OPTARG
            ;;
        ?)
            cat << '_EOF_'
Usage: 
    cecho [-niswebu] [-fg color] [-L url] [arg ...]

Options:
    -n	do not append a newline
    -i  info is light cyan text
    -s  success is green text
    -w  warning is yellow text
    -e  error is red text
    -b  bold text
    -u  underlined text
    -f  set foreground color
    -g  set background color
    -L  set hyperlink
_EOF_
            return 1
            ;;
        esac
    done
    shift "$(($OPTIND - 1))"

    if [ $HYPERLINK ]; then
        printf "\e]8;;%s\e\\%s\e]8;;\e\\" $HYPERLINK $*
        tput sgr0
        [ $DONT_APPEND_NEWLINE ] || printf "\n"
    else
        echo $DONT_APPEND_NEWLINE "$@"`tput sgr0`
    fi
    return 0
}

cecho "$@"

#!/usr/bin/env bash

set -e

# usage: sgrcolor <color>
function sgrcolor() {
    local -a COLORS=(black red green yellow blue magenta cyan white)
    for i in ${!COLORS[@]}; do
        if [[ ${COLORS[i]} == $1 ]]; then
            echo $i
            return
        fi
    done
    [[ $1 == 'all' ]] && echo ${COLORS[*]} || echo $1
}

# cecho - Write arguments to the standard output
function cecho() {
    local OPTION OPTARG OPTIND
    while getopts ':nISWEBUf:b:u:' OPTION; do
        case "$OPTION" in
        n)
            local DONT_APPEND_NEWLINE="\c"
            ;;
        I)
            tput bold
            tput setaf "$(sgrcolor cyan)"
            ;;
        S)
            tput bold
            tput setaf "$(sgrcolor green)"
            ;;
        W)
            tput bold
            tput setaf "$(sgrcolor yellow)"
            ;;
        E)
            tput bold
            tput setaf "$(sgrcolor red)"
            ;;
        B)
            tput bold
            ;;
        U)
            tput smul
            ;;
        f)
            tput setaf "$(sgrcolor $OPTARG)"
            ;;
        b)
            tput setab "$(sgrcolor $OPTARG)"
            ;;
        u)
            local HYPERLINK=$OPTARG
            ;;
        ?)
            tput sgr0
            cat << _EOF_
$0: illegal option -- $OPTARG

Usage: 
    cecho [-nISWEBU] [-fb color] [-u url] [arg ...]

Options:
    -n	        do not append a newline
    -I          info is light cyan text
    -S          success is green text
    -W          warning is yellow text
    -E          error is red text
    -B          bold text
    -U          underlined text
    -f <color>  set foreground color
    -b <color>  set background color
    -u <url>    set hyperlink url

Color:
    $(sgrcolor all)

_EOF_
            return 1
            ;;
        esac
    done
    shift "$(($OPTIND - 1))"

    if [ $HYPERLINK ]; then
        printf "\e]8;;%s\a%s\e]8;;\a" $HYPERLINK $*
        tput sgr0
        [ $DONT_APPEND_NEWLINE ] || printf "\n"
    else
        if [[ "$(echo \\)" != '\' ]]; then
            local OPT_BACKSLASH_ESC='-e'
        fi
        echo $OPT_BACKSLASH_ESC "$@${DONT_APPEND_NEWLINE}"`tput sgr0`
    fi
    return 0
}

cecho "$@"

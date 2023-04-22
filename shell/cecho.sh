#!/usr/bin/env bash

set -e

# usage: sgrcolor <color>
function sgrcolor() {
    local -a COLORS=(black red green yellow blue magenta cyan white)
    local -i i=0
    for c in ${COLORS[@]}; do
        if [[ $c == $1 ]]; then
            echo $i
            return
        fi
        ((++i))
    done
    [[ $1 == 'all' ]] && echo ${COLORS[*]} || echo $1
}

# cecho - Write arguments to the standard output
function cecho() {
    local OPTION OPTARG OPTIND
    while getopts ':nISWEBUCf:b:u:' OPTION; do
        case "$OPTION" in
        n)
            if [[ -z "$(echo -n)" ]]; then
                local -r OPT_DONT_APPEND_NEWLINE='-n'
            fi
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
        C)
            local -ri CLEAR_END_OF_LINE=1
            ;;
        f)
            tput setaf "$(sgrcolor $OPTARG)"
            ;;
        b)
            tput setab "$(sgrcolor $OPTARG)"
            ;;
        u)
            local -r HYPERLINK=$OPTARG
            ;;
        ?)
            tput sgr0
            cat >&2 << _EOF_
$0: illegal option -- $OPTARG

Usage: 
    cecho [-nISWEBUC] [-fb color] [-u url] [arg ...]

Options:
    -n	        do not append a newline
    -I          info is light cyan text
    -S          success is green text
    -W          warning is yellow text
    -E          error is red text
    -B          bold text
    -U          underlined text
    -C          clear to end of line with background color
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

    local DATA=''
    if [[ $# -eq 0 && -s /dev/stdin || -f $1 ]]; then
        local -r IFS=
        while read -r line; do
            [[ $DATA || -z $line ]] && DATA+="\n"
            DATA+=$line
        done < ${1:-/dev/stdin}
    else
        DATA=$@
    fi

    [[ $CLEAR_END_OF_LINE ]] && tput el
    if [ $HYPERLINK ]; then
        printf "\e]8;;%s\a%s\e]8;;\a" $HYPERLINK "$DATA"
        tput sgr0
        [ $OPT_DONT_APPEND_NEWLINE ] || printf "\n"
    else
        if [[ -z "$(echo -e)" ]]; then
            local -r OPT_BACKSLASH_ESC='-e'
        fi
        echo $OPT_DONT_APPEND_NEWLINE $OPT_BACKSLASH_ESC "${DATA}"`tput sgr0`
    fi
    return 0
}

# cecho "$@"

#!/usr/bin/env bash

set -e

# usage: sgrcolor <color>
function sgrcolor() {
    local -a COLORS=(black red green yellow blue magenta cyan white)
    local -i i=0
    for C in "${COLORS[@]}"; do
        if [[ $C == "$1" ]]; then
            echo $i
            return
        fi
        ((++i))
    done
    [[ $1 == 'all' ]] && echo "${COLORS[*]}" || echo "$1"
}

# cecho - Write arguments to the standard output
function cecho() {
    local OPTION OPTARG OPTIND
    while getopts ':nISWEBUCf:b:u:' OPTION; do
        case "$OPTION" in
        n)
            local -r DONT_APPEND_NEWLINE='\c'
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
            tput setaf "$(sgrcolor "$OPTARG")"
            ;;
        b)
            tput setab "$(sgrcolor "$OPTARG")"
            ;;
        u)
            local -r HYPERLINK="$OPTARG"
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
    shift $((OPTIND - 1))

    # for hyperlink
    [[ -n "$CLEAR_END_OF_LINE" ]] && tput el
    if [[ -n "$HYPERLINK" ]]; then
        printf "\e]8;;%s\a%s\e]8;;\a" "$HYPERLINK" "$*"
        tput sgr0
        [[ -n "$DONT_APPEND_NEWLINE" ]] || printf "\n"
        return 0
    fi

    if [[ -z "$(echo -e)" ]]; then
        local -r OPT_BACKSLASH_ESC='-e'
    fi

    # for heredoc or file
    if [[ $# -eq 0 && ! -t 0 || -f $1 ]]; then
        local IFS=
        local PREV_LINE
        local CURR_LINE
        local -i CALL_ONCE=0
        while read -r CURR_LINE; do
            if [[ $CALL_ONCE -eq 0 ]]; then
                CALL_ONCE=1
            else
                [[ -n "$CLEAR_END_OF_LINE" ]] && tput el
                echo $OPT_BACKSLASH_ESC "${PREV_LINE}${DONT_APPEND_NEWLINE}"
            fi
            PREV_LINE="$CURR_LINE"
        done < "${1:-/dev/stdin}"
        [[ -n "$CLEAR_END_OF_LINE" ]] && tput el
        echo $OPT_BACKSLASH_ESC "${PREV_LINE}$(tput sgr0)${DONT_APPEND_NEWLINE}"
        return 0
    fi

    # for string
    echo $OPT_BACKSLASH_ESC "$*$(tput sgr0)${DONT_APPEND_NEWLINE}"
    return 0
}

cecho "$@"
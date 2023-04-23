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
    local -ri TPUT_ENABLE=$(test -t 2 && which tput > /dev/null 2>&1 && echo 1 || echo 0)
    local OPTION OPTARG OPTIND
    while getopts ':nISWEBUCf:b:u:' OPTION; do
        case "$OPTION" in
            n)
                local -r DONT_APPEND_NEWLINE='\c'
                ;;
            I)
                local -ri SGR_BOLD=1
                local -r SGR_FOREGROUND_COLOR='cyan'
                ;;
            S)
                local -ri SGR_BOLD=1
                local -r SGR_FOREGROUND_COLOR='green'
                ;;
            W)
                local -ri SGR_BOLD=1
                local -r SGR_FOREGROUND_COLOR='yellow'
                ;;
            E)
                local -ri SGR_BOLD=1
                local -r SGR_FOREGROUND_COLOR='red'
                ;;
            B)
                local -ri SGR_BOLD=1
                ;;
            U)
                local -ri SGR_UNDERLINE=1
                ;;
            C)
                local -ri CLEAR_END_OF_LINE="$TPUT_ENABLE"
                ;;
            f)
                local -r SGR_FOREGROUND_COLOR="$OPTARG"
                ;;
            b)
                local -r SGR_BACKGROUND_COLOR="$OPTARG"
                ;;
            u)
                local -r HYPERLINK="$OPTARG"
                ;;
            ?)
                [[ "$TPUT_ENABLE" -eq 1 ]] && tput sgr0
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
    
    # set graphics mode
    if [[ $TPUT_ENABLE -eq 1 ]]; then
        tput sgr "${SGR_STDOUT:-0}" \
        "${SGR_UNDERLINE:-0}"       \
        "${SGR_REVERSE:-0}"         \
        "${SGR_BLINK:-0}"           \
        "${SGR_DIM:-0}"             \
        "${SGR_BOLD:-0}"            \
        "${SGR_INVIS:-0}"           \
        "${SGR_PROTECT:-0}"         \
        "${SGR_ALTCHARSET:-0}"
        
        [[ -n "$SGR_BACKGROUND_COLOR" ]] && tput setab "$(sgrcolor "$SGR_BACKGROUND_COLOR")"
        [[ -n "$SGR_FOREGROUND_COLOR" ]] && tput setaf "$(sgrcolor "$SGR_FOREGROUND_COLOR")"
        
        local -r SGR_RESET=$(tput sgr0)
    fi
    
    # for hyperlink
    if [[ -n "$HYPERLINK" ]]; then
        [[ "$CLEAR_END_OF_LINE" -eq 1 ]] && tput el
        printf "\e]8;;%s\a%s\e]8;;\a${SGR_RESET}" "$HYPERLINK" "$*"
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
                [[ "$CLEAR_END_OF_LINE" -eq 1 ]] && tput el
                echo $OPT_BACKSLASH_ESC "${PREV_LINE}${DONT_APPEND_NEWLINE}"
            fi
            PREV_LINE="$CURR_LINE"
        done < "${1:-/dev/stdin}"
        [[ "$CLEAR_END_OF_LINE" -eq 1 ]] && tput el
        echo $OPT_BACKSLASH_ESC "${PREV_LINE}${SGR_RESET}${DONT_APPEND_NEWLINE}"
        return 0
    fi
    
    # for string
    [[ "$CLEAR_END_OF_LINE" -eq 1 ]] && tput el
    echo $OPT_BACKSLASH_ESC "$*${SGR_RESET}${DONT_APPEND_NEWLINE}"
    return 0
}

if [[ $# -gt 0 ]]; then
    cecho "$@"
fi

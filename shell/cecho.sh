#!/usr/bin/env bash

# usage: sgrcolor <color>
function sgrcolor() {
    local -a COLORS=(black red green yellow blue magenta cyan white)
    local -i i=0
    case "$1" in
        info)
            local -r SEL_COLOR='cyan'
            ;;
        succ)
            local -r SEL_COLOR='green'
            ;;
        warn)
            local -r SEL_COLOR='yellow'
            ;;
        error)
            local -r SEL_COLOR='red'
            ;;
        *)
            local -r SEL_COLOR="$1"
            ;;
    esac
    for C in "${COLORS[@]}"; do
        if [[ $C == "$SEL_COLOR" ]]; then
            echo $i
            return
        fi
        ((++i))
    done
    [[ $1 == 'all' ]] && echo "${COLORS[*]}" || echo "$1"
}

# setaf - set ANSI foreground color
# usage: setaf <color>
function setaf() {
    if [[ -t 2 ]]; then
        which tput >/dev/null 2>&1
        [[ $? -eq 0 ]] && tput setaf $(sgrcolor "$1")
    fi
}

# usage: hyperlink <url> <text>
function hyperlink() {
    local -r URL="$1"
    shift
    if [[ ! -t 2 || $# -lt 1 ]]; then
        echo "$*"
        return 0
    fi
    which tput >/dev/null 2>&1
    if [[ $? != 0 ]]; then
        echo "$*"
        return 0
    fi
    printf "\e]8;;%s\a%s\e]8;;\a" "$URL" "$*"
}

# cecho - Write arguments to the standard output
function cecho() {
    if [[ -t 2 ]]; then
        which tput >/dev/null 2>&1
        local -ri TPUT_ENABLE=!$?
    fi
    local -i TFRAME_MAX_COLS=79
    if [[ $TPUT_ENABLE -eq 1 && $(tput cols) -lt $TFRAME_MAX_COLS ]]; then
        local -i TFRAME_MAX_COLS=$(tput cols)
    fi
    local OPTION OPTARG OPTIND
    while getopts ':nISWEBUCf:b:u:Hh:Vv:' OPTION; do
        case "$OPTION" in
            n)
                local -r DONT_APPEND_NEWLINE='\c'
                ;;
            I)
                local -ri SGR_BOLD=1
                local -r SGR_FOREGROUND_COLOR='info'
                ;;
            S)
                local -ri SGR_BOLD=1
                local -r SGR_FOREGROUND_COLOR='succ'
                ;;
            W)
                local -ri SGR_BOLD=1
                local -r SGR_FOREGROUND_COLOR='warn'
                ;;
            E)
                local -ri SGR_BOLD=1
                local -r SGR_FOREGROUND_COLOR='error'
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
            H)
                local -ri TFRAME_HLINE=$TFRAME_MAX_COLS
                ;;
            h)
                if [[ $OPTARG -lt $TFRAME_MAX_COLS ]]; then
                    local -ri TFRAME_HLINE=$OPTARG
                else
                    local -ri TFRAME_HLINE=$TFRAME_MAX_COLS
                fi
                ;;
            V)
                local -ri TFRAME_VLINE=$TFRAME_MAX_COLS
                ;;
            v)
                if [[ $OPTARG -lt $TFRAME_MAX_COLS ]]; then
                    local -ri TFRAME_VLINE=$OPTARG
                else
                    local -ri TFRAME_VLINE=$TFRAME_MAX_COLS
                fi
                ;;
            ?)
                [[ $TPUT_ENABLE -eq 1 ]] && tput sgr0
                cat >&2 << _EOF_
$0: illegal option -- $OPTARG

Usage:
    cecho [-nISWEBUC] [-fb color] [-u url] [-H|-h cols] [-V|-v cols] [arg ...]

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
    -H | -h <cols>  draw text frame horizontal line
    -V | -v <cols>  draw text surrounded by text frame

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

    if [[ -z "$(echo -e)" ]]; then
        local -r OPT_BACKSLASH_ESC='-e'
    fi

    # for hyperlink
    if [[ $TPUT_ENABLE -eq 1 && -n "$HYPERLINK" ]]; then
        [[ $CLEAR_END_OF_LINE -eq 1 ]] && tput el
        local -r HL_TXT=$(hyperlink "$HYPERLINK" "$*")
        echo $OPT_BACKSLASH_ESC "${HL_TXT}${SGR_RESET}${DONT_APPEND_NEWLINE}"
        return 0
    fi

    # for heredoc or file
    if [[ $# -eq 0 && ! -t 0 || -f $1 ]]; then
        local PREV_LINE
        local CURR_LINE
        local -i CALL_ONCE=0
        while IFS= read -r CURR_LINE; do
            if [[ $CALL_ONCE -eq 0 ]]; then
                CALL_ONCE=1
            else
                [[ $CLEAR_END_OF_LINE -eq 1 ]] && tput el
                echo $OPT_BACKSLASH_ESC "${PREV_LINE}${DONT_APPEND_NEWLINE}"
            fi
            PREV_LINE="$CURR_LINE"
        done < "${1:-/dev/stdin}"
        if [[ $CALL_ONCE != 0 ]]; then
            [[ $CLEAR_END_OF_LINE -eq 1 ]] && tput el
            echo $OPT_BACKSLASH_ESC "${PREV_LINE}${SGR_RESET}${DONT_APPEND_NEWLINE}"
            return 0
        fi
    fi
    
    [[ $CLEAR_END_OF_LINE -eq 1 ]] && tput el
    # for string
    if [[ -z "$TFRAME_HLINE" && -z "$TFRAME_VLINE" ]]; then
        echo $OPT_BACKSLASH_ESC "$*${SGR_RESET}${DONT_APPEND_NEWLINE}"
        return 0
    fi

    # for text frame horizontal line
    if [[ $# -eq 0 ]]; then
        local -ri TFRAME_DASHS=${TFRAME_HLINE}-2
        echo $OPT_BACKSLASH_ESC "+$(printf '\055%.0s' $(seq "$TFRAME_DASHS"))+${SGR_RESET}"
        return 0
    fi

    # for text frame text
    local -ri RESERVED_SIZE=${TFRAME_VLINE}-2
    local -r RESERVED_TEXT="$(printf '\x20%.0s' $(seq "$RESERVED_SIZE"))"

    local -r EXPR_REPLACE_TAB="s/\t/\x20\x20\x20\x20\x20\x20/g"
    local -r EXPR_ERASE_ANSI='s/\x1b\[[0-9;]*[a-zA-Z]//g'
    local -r EXPR_ERASE_HYPERLINK='s/\x1b\]8;;\(.*\)\x07\(.*\)\x1b\]8;;\x07/\2/g'

    local TEXT=$(echo $OPT_BACKSLASH_ESC "$*" | sed -e "$EXPR_REPLACE_TAB")
    local -r TRIPED_TEXT=$(echo $OPT_BACKSLASH_ESC "$TEXT" | sed -e "$EXPR_ERASE_ANSI" -e "$EXPR_ERASE_HYPERLINK")

    local TEXT="${TEXT}${RESERVED_TEXT:${#TRIPED_TEXT}}"
    if [[ $TPUT_ENABLE -eq 1 ]]; then
        tput hpa 0
        if [[ $? -eq 0 ]]; then
            echo $OPT_BACKSLASH_ESC "|${TEXT}$(tput hpa $((TFRAME_VLINE-1)))|${SGR_RESET}"
            return 0
        fi
    fi
    local TEXT=$(echo "$TEXT" | sed -e "$EXPR_ERASE_HYPERLINK")
    echo $OPT_BACKSLASH_ESC "|${TEXT}|${SGR_RESET}"
    return 0
}

# puts - print status string
function puts() {
    local OPTION OPTARG OPTIND
    while getopts 'c:l:' OPTION; do
        case "$OPTION" in
            c)
                local -r COLOR="$OPTARG"
                ;;
            l)
                local -r LABEL="$OPTARG"
                ;;
            ?)
                cecho "usage: puts [-c <color>] [-l <label>] [action] [arg...]"
                ;;
        esac
    done
    shift $((OPTIND - 1))

    cecho -n -f "$COLOR" -- "${LABEL:-==>}\x20"
    if [[ $# == 1 ]]; then
        cecho -B -- "$1"
    elif [[ $# -gt 1 ]]; then
        cecho -n -B -- "$1\x20"
        shift
        cecho -B -f "$COLOR"  -- "$*"
    fi
}

if [[ -n "$TFRAME_RUN_EXAMPLE" ]]; then
    declare -r CMAKE_VER=3.0.2
    # text frame top line
    cecho -H -I -b white
    while IFS= read -r CURR_LINE; do
        cecho -V -I -b white -- "$CURR_LINE"
    done <<- _EOF_

    $(hyperlink https://github.com/tdlib/td TDLib) depends on:
        - $(setaf warn)C++14 compatible compiler$(setaf info)
        - OpenSSL
        - zlib
        - $(setaf red)gperf (build only)$(setaf info)
        - CMake (${CMAKE_VER}+, build only)
        - PHP (optional, for documentation generation)

_EOF_
    # text frame bottomline
    cecho -H -I -b white
fi

if [[ -n "$RUN_CECHO" ]]; then
    cecho "$@"
fi

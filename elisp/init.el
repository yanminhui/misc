;; .emacs.d/init.el

;; ===================================
;; MELPA Package Support
;; ===================================

;; Enables basic packaging support
(require 'package)

;; Adds the Melpa archive to the list of available repositories
(setq package-archives
      '(("org" . "http://mirrors.tuna.tsinghua.edu.cn/elpa/org/")
        ("gnu" . "http://mirrors.tuna.tsinghua.edu.cn/elpa/gnu/")
        ("melpa-stable"
         . "http://mirrors.tuna.tsinghua.edu.cn/elpa/melpa-stable/")
        ("melpa" . "http://mirrors.tuna.tsinghua.edu.cn/elpa/melpa/")))

;; Initializes the package infrastructure
(package-initialize)

;; If there are no archived package contents, refresh them
(when (not package-archive-contents) (package-refresh-contents))

;; Installs packages
;;
;; myPackages contains a list of package names
(defvar myPackages
  '(better-defaults                 ;; Set up some better Emacs defaults
    python-black                    ;; Black formatting on save (blacken)
    company                         ;; Completion anything
    company-c-headers               ;; Company backends
    cmake-mode                      ;; CMake build system
    ein                             ;; Emacs IPython Notebook
    elpy                            ;; Emacs Lisp Python Environment
    flycheck                        ;; On the fly syntax checking
    groovy-mode                     ;; Groovy language
    projectile                      ;; Project Manager
    py-autopep8                     ;; Run autopep8 on save
    material-theme                  ;; Theme
    yasnippet                       ;; Yet Another Snippet
    yasnippet-snippets              ;; Yet Another Snippet's snippets
    ))

;; Scans the list in myPackages
;; If the package listed is not already installed, install it
(mapc (lambda (package)
        (unless (package-installed-p package)
          (package-install package)))
      myPackages)

;; ===================================
;; Basic Customization
;; ===================================

(setq inhibit-startup-message t)    ;; Hide the startup message
(load-theme 'material t)            ;; Load material theme
(global-linum-mode t)               ;; Enable line numbers globally
(column-number-mode t)              ;; Enable column numbers globally

;; ====================================
;; Development Setup
;; ====================================

;; Default style
(setq c-default-style
      '((c++-mode  . "stroustrup")
        (java-mode . "java")
        (awk-mode  . "awk-mode")
        (other     . "gnu")))

;; Program mode hook
(add-hook 'prog-mode-hook 'show-paren-mode)

(require 'yasnippet)
(add-hook 'prog-mode-hook 'yas-minor-mode)

(add-hook 'prog-mode-hook
          (lambda()
            (setq indent-tabs-mode nil) ;; Convert tabs to multiple spaces
            ))

;; C mode common  hook
(require 'projectile)
(define-key projectile-mode-map
  (kbd "C-c p") 'projectile-command-map)  ;; Project manager prefix
(add-hook 'c-mode-common-hook 'projectile-mode)

(require 'company)
(add-to-list 'company-backends 'company-c-headers)
(add-hook 'c-mode-common-hook 'company-mode)

;; C++ mode hook
(add-hook 'c++-mode-hook
          (lambda()
            (setf (alist-get 'inline-open c-offsets-alist) 0)
            (setf (alist-get 'innamespace c-offsets-alist) 0)
            (setf (alist-get 'statement-cont c-offsets-alist) 0)
            ))

;; Enable elpy
(elpy-enable)

;; Use IPython for REPL (Read Eval Print Loop)
(setq python-shell-interpreter "ipython"
      python-shell-interpreter-args "-i --simple-prompt")

;; Enable Flycheck
(when (require 'flycheck nil t)
  (setq elpy-modules (delq 'elpy-module-flymake elpy-modules))
  (add-hook 'elpy-mode-hook 'flycheck-mode))

;; Enable autopep8
(require 'py-autopep8)
(add-hook 'elpy-mode-hook 'py-autopep8-enable-on-save)
(custom-set-variables
 ;; custom-set-variables was added by Custom.
 ;; If you edit it by hand, you could mess it up, so be careful.
 ;; Your init file should contain only one such instance.
 ;; If there is more than one, they won't work right.
 '(package-selected-packages
   (quote
    (org-link-minor-mode yasnippet-snippets python-black py-autopep8 projectile material-theme groovy-mode flycheck elpy ein company-c-headers cmake-mode better-defaults))))
(custom-set-faces
 ;; custom-set-faces was added by Custom.
 ;; If you edit it by hand, you could mess it up, so be careful.
 ;; Your init file should contain only one such instance.
 ;; If there is more than one, they won't work right.
 )

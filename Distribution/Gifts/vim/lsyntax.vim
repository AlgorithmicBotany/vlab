au  BufNewFile,BufRead,BufWrite *.l if match(getline(1,20), 'lpfgall.h') >= 0 | set filetype=lpfg | else | set filetype=cpfg | endif

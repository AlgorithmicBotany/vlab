if [ -f lsys.so ]; then
  rm lsys.so
fi

if [ -f lpfg.mak ]; then
  make -f lpfg.mak
else
  make -f $VLABCONFIGDIR/lpfg.mak
fi

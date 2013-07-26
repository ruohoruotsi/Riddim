plot(diffsig(maxid+riseids))
t = 1:11;
plot(t, diffsig(maxid+riseids),t, slopegauss(round(timefracdec/2)+1+riseids) )
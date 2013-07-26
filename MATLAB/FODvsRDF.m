load FODvsRDF.mat
y2 = diff(y1);
y3 = diff(log(y1));
plot(t, y1(1:400), t, y2, t, y3);



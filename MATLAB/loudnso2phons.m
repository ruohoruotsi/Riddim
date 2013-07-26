function so2phons = loudnso2phons(sones)
npts = 33;

%* table of log2(sones)

logsones=[-50.0, -20.0000, -19.7842, -13.0253,...
      -9.9484,  -7.0087,  -5.5895,  -3.9409,  -2.8241,...
      -1.9572,  -1.2290,  -0.5861,   0.0000,   0.5468,...
      1.0658,   1.5663,   2.0543,   2.5356,   3.0159,...
      3.5011,   4.0000,   4.5278,   5.0643,   5.6039,...
      6.1517,   6.7112,   7.2842,   7.8708,   8.4690,...
      9.0731,   9.6699,  10.2213,  10.6025];

%* matching table of phons
stdphons=[4.2,      4.2,      4.205,    4.3,...
      5.0,      7.5,     10.0,     15.0,     20.0,...
      25.0,     30.0,     35.0,     40.0,     45.0,...
      50.0,     55.0,     60.0,     65.0,     70.0,...
      75.0,     80.0,     85.0,     90.0,     95.0,...
      100.0,    105.0,    110.0,    115.0,    120.0,...
      125.0,    130.0,    135.0,    140.0];


lsones = log10(sones)/log10(2);
npol = 3;
index = loudnlocate(logsones,npts,lsones);
if index <= 2
  npol = 2;
end
index = min(max(index-(npol-1)/2,1),npts+1-npol);
if index==0
  so2phons = 0;
  return
end

%*	write(6,'('' index'',i2,'' lsones '',f9.4)')index,lsones

if((index==npts)|(lsones > 10.6025))
  so2phons = 999.9;
  return
end
index=ceil(index);
[so2phons,dy] = loudnpolint( logsones(index:index+npol-1),...
    stdphons(index:index+npol-1),npol,lsones);
%	end

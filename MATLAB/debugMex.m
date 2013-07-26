% debug this mofo.
load iroro_initialSignal.mat      
figure;
plot(iroro_debug_variable);

load iroro_gaussenv.mat           
figure;
plot(iroro_debug_variable);

load io_after_rectify.mat         
figure;
plot(iroro_debug_variable);

load io_after_thresholding.mat
figure;
plot(iroro_debug_variable);

load io_after_decFiiltering.mat
figure;
plot(iroro_debug_variable);

load iroro_debug_b4convdecimated.mat       
figure;
plot(iroro_debug_variable);

load iroro_debug_afterconv.mat    
figure;
plot(iroro_debug_variable);



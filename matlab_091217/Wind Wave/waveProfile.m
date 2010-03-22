%
% Mathematical model of the movement of the ship in the water
% with influence of wind, current and waves
%
% - Wave profile generator
%
% (c) by Fabian Jenne


%  DAsoln3;  compute the D'Almebert soln for an IVP.
%  The original disturbance is Gaussian, so we know the 
%  Fourier transform (also a Gaussian).  We can assume a 
%  dispersion relation that is normal, non-dispersive or  
%  anomalous (the most intersting one!).
%
%  Public domain for all educational uses.
%
%  Jim Price, January 2000

clear
close all
format compact

%  choose whether this system is dispersive or not
dispers = menu('Choose your dispersion relation', ...
   'anomalous', 'non-dispersive', 'normal', ... 
   'very strange, frequency = const')
dispers = dispers - 2;

%  let's choose f(x) to be Gaussian 

x = -100:0.1:100;              %  the spatial domain
alpha = 0.1;
f = exp(-(x.^2)*alpha);      %  define the function in space

%  reconstruct f(x) from its (known) transform, F

dk = 0.01;               %  the wavenumber resolution
k = -4:dk:4;             %  the wavenumber domain
[nn nk] = size(k);

Sizey = pi/dk

% the known, continuous Fourier transform

F = (1/sqrt(4*pi*alpha))*exp(-(k.^2)/(4*alpha));

y = -150:1:150;            %  invert on this spatial domain
[qq yn] = size(y);

%  compute the inverse transform, fy, on the domain y

for m=1:yn
  fy(m) = sum(F.*real(exp(-i*k*y(m)))*dk);
end

%  Set some default graphics things.
set(0,'DefaultTextFontSize',12)
set(0,'DefaultAxesFontSize',12)
set(0,'DefaultAxesLineWidth',1.6)
set(0,'DefaultLineLineWidth',1.4)


%  plot the function, the spectrum and the inverse transform 

figure(1)
clf reset

subplot(2,1,1)
plot(x, f, y, real(fy), 'r+')
xlabel('x distance')
ylabel('function')
title('Fourier transform and inverse')

subplot(2,1,2)
plot(k, F)
xlabel('wavenumber, k')
ylabel('spectral amplitude, x^2/dk')

%  Now do an initial value problem with this function as the IC

%  anomalous dispersion
if dispers == -1
for j=1:nk
   C(j) = sqrt(abs(k(j)));
   if C(j) >= 10; C(j) = 10.; end
end
titl = 'anomalous dispersion'
end

%  non-dispersive
if dispers == 0
for j=1:nk          %  compute a reasonable mean C
C(j) = sqrt(1./abs(k(j)));
if C(j) >= 10; C(j) = 10.; end
end
Csa = mean(abs(F).*C)/mean(abs(F))
C = 0*C + Csa;
titl = 'non-dispersive waves'
end

% a deepwater-like C(k) (g = 1)
if dispers == 1
for j=1:nk
   C(j) = sqrt(1./abs(k(j)));
   if C(j) >= 10; C(j) = 10.; end
end
titl = '(normal) dispersive waves'
end

%  very strange dispersion; constant frequency
if dispers == 2
for j=1:nk
   C(j) = abs(1/k(j));
if C(j) >= 1000; C(j) = 1000; end
end
titl = 'very strange dispersion; frequency = 1 and constant'
end

omega = abs(k.*C);    %  from C, compute the wavenumbers

figure(4)
clf reset
subplot(2,2,1)
plot(k, omega)
xlabel('k'); ylabel('\omega')
subplot(2,2,2)
plot(k, C)
xlabel('k'); ylabel('C')


%  compute the time-shifted (a la D'Alembert) inverse of F
dt = 1.0;
tend = 50.;
if dispers >= 0; dt = 0.5; end
t = 0:dt:tend;          %  evaluate for these times
[qq tn] = size(t);

for m=1:tn   
for n=1:yn
  yD(m,n) = real(sum( dk*F.*exp(-i*(k*y(n) - omega*t(m))) ));
end

thisfar = t(m)/tend   %  just so you know it's alive
end


%  check for energy and mass conservation.
vy = var(yD');
ay = mean(yD');

figure(4)
subplot(2,2,3)
plot(t, vy)
ylabel('var y')
xlabel('time')
subplot(2,2,4)
plot(t, ay)
ylabel('avg y')
xlabel('time')
drawnow

%  contour the field, y
dothis = 0;
if dothis == 1
figure(2)
clf reset
wc = contour(y, t, yD, [-0.6:0.2:1.]);
clabel(wc);
xlabel('distance')
ylabel('time')
title('Fourier tranform and D`Alembert`s solution')
drawnow
end

%  a 3-D surface
figure(3)
clf reset
mesh(y, t, yD);
view(-40., 40.);
axis([-150 150 0 50 -0.2 0.6])
xlabel('distance')
ylabel('time')
zlabel('amplitude')
title(titl)
drawnow

ddd = '  Hit any key to continue with a movie.'
pause

%  make a movie
figure(5)
clf reset
for j=1:tn   
f1 = yD(j,:);
plot(y, f1)
axis([-150 150 -0.4 1.0])
time = floor(dt*j);
timstr = ['time = ', num2str(time)];
text(0., 0.6, timstr)
if dispers == -1
   text(0., 0.7,'anomalous dispersion');
end
if dispers == 0
   text(0., 0.7,'non-dispersive')
end
if dispers == 1
   text(0., 0.7,'normal dispersion')
end

drawnow
M(j) = getframe;
end
movie(M,6,2)



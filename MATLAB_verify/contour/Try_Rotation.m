clc
clear
close all

%%
% get model point cloud 
load('back_2_.mat');
MAX = max(abs(data(:)));
X = data(:,1)./MAX;
Y = data(:,2)./MAX;
Z = data(:,3)./MAX;

%avoid negative & move to original coordinate
X = X - min(X);
Y = Y - min(Y);
Z = Z - min(Z);

x_floor = min([min(X(:)),min(Y(:)),min(Z(:))]);
x_ceil  = max([max(X(:)),max(Y(:)),max(Z(:))]);

figure,plot(X,Y,'.'); title('front');
set(gca, 'XLim', [x_floor x_ceil]);
set(gca, 'YLim', [x_floor x_ceil]);

figure,scatter3(X,Y,Z,5,[0 0 1],'.'); view([90,90,90]);drawnow;



%%
%Rotate
alpha = pi/4;
R_x = [1               0                0         ;
       0               cos(alpha)       sin(alpha);
       0              -sin(alpha)       cos(alpha)];

beta = 0;%pi/4;
R_y = [cos(beta)       0            -sin(beta);
       0               1             0        ;
       sin(beta)       0             cos(beta)];
R = R_x*R_y
X_size = length(X);
for i = 1:X_size
    x = X(i);
    y = Y(i);
    z = Z(i);
    temp = [x y z]*R;
    X(i) = temp(1);
    Y(i) = temp(2);
    Z(i) = temp(3);    
end

figure,plot(X,Y,'.'); title('front');
set(gca, 'XLim', [x_floor x_ceil]);
set(gca, 'YLim', [x_floor x_ceil]);

figure,scatter3(X,Y,Z,5,[0 0 1],'.'); view([90,90,90]);drawnow;
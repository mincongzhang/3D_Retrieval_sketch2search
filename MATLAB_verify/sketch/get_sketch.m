clc
clear
close all

n_sketches = 5;

for i = 1:n_sketches
    [X(i) Y(i)]=ginput(1);hold on;   
    plot(X,Y,'-o');title('original sketch');hold on;drawnow;
    set(gca, 'XLim', [0.0 1.0]);
    set(gca, 'YLim', [0.0 1.0]);
end

%%
%DRAWBACK: cannot recognize different size shape: 
%e.g. small square v.s. big square

%avoid negative & move to original coordinate
X = X - min(X);
Y = Y - min(Y);

% X = X./max(X); %DRAWBACK: the shape will be stretched
% Y = Y./max(Y);
norm_factor = max([max(X(:)),max(Y(:))]);
X = X./norm_factor;
Y = Y./norm_factor;

%%
%interpolate
X_interpolate = [X(1)];
Y_interpolate = [Y(1)];

for i = 2:n_sketches
    vector_x = X(i)-X(i-1);
    vector_y = Y(i)-Y(i-1);
    dist = floor(norm([vector_x,vector_y])*50);
    %NEED FURTHER CONSISERATION!
    %if I draw a small square, cannot interlolate many points according to
    %distance -> solution: normalize (step 1)
    for j = 1:dist
        X_interpolate(end+1) = X(i-1)+j*vector_x/dist;
        Y_interpolate(end+1) = Y(i-1)+j*vector_y/dist;
    end
end
figure;plot(X_interpolate,Y_interpolate,'o'); title('interpolated and normalized sketch');
set(gca, 'XLim', [0.0 1.0]);
set(gca, 'YLim', [0.0 1.0]);
% length(X_interpolate)

%%
%map to 32x32 Grid
grid = zeros(32,32);
X_interpolate = round(X_interpolate.*32);
Y_interpolate = round(Y_interpolate.*32);
n_points = length(X_interpolate);

% Grid (to avoid double count sketch point in the same grid)
for i = 1:n_points
    grid(Y_interpolate(i)+1,X_interpolate(i)+1) = 1; % avoid coordinate = 0, after normalize the sketch
end

[Y_grid,X_grid] = find(grid == 1);

%%
% get centroid in 32x32 grid
% NEED FURTHER CONDISERATION!
% centroid of a polygon is the mean value of all the coordintates?
x_mean = mean(X_grid);
y_mean = mean(Y_grid);

grid_length = length(X_grid(:));
for i = 1:grid_length-1
    edge1 = [X_grid(i)-x_mean,Y_grid(i)-y_mean];
    edge2 = [X_grid(i+1)-x_mean,Y_grid(i+1)-y_mean];
    cross_area = cross([edge1,0],[edge2,0]);
    abs_area(i) = abs(cross_area(3));
    tri_centroid_x(i) = mean([X_grid(i),X_grid(i+1),x_mean]);
    tri_centroid_y(i) = mean([Y_grid(i),Y_grid(i+1),y_mean]);
end

%coordinates in 32x32 grid
centroid_x = mean(tri_centroid_x);
centroid_y = mean(tri_centroid_y);
% plot(centroid_x,centroid_y,'*');hold on;
% plot(x_mean,y_mean,'r*');hold on;
%%
%histogram
for i = 1:grid_length
    diff(i) = sqrt((X_grid(i)-centroid_x)^2 + (Y_grid(i)-centroid_y)^2);
end
diff = diff./(32*sqrt(2));
hist_test = zeros(ceil(100*sqrt(2)),1);

for i = 1:grid_length
    hist_test(round(diff(i).*100)) = hist_test(round(diff(i).*100)) +1;
end

figure,plot(hist_test); title('histogram')

%%
% %similarity
load('hist_circle.mat');
similarity_circle = sum(hist_test.*hist)/(norm(hist_test)*norm(hist))
load('hist_square.mat');
similarity_square = sum(hist_test.*hist)/(norm(hist_test)*norm(hist))
load('hist_tri.mat');
similarity_tri = sum(hist_test.*hist)/(norm(hist_test)*norm(hist))
load('hist_halfcircle.mat');
similarity_halfcircle = sum(hist_test.*hist)/(norm(hist_test)*norm(hist))
load('back_2_hist_front.mat');
similarity_front_view_of_chair_back_2 = sum(hist_test.*hist_front)/(norm(hist_test)*norm(hist_front))
load('back_6_hist_front.mat');
similarity_front_view_of_chair_back_6 = sum(hist_test.*hist_front)/(norm(hist_test)*norm(hist_front))
load('back_36_hist_front.mat');
similarity_front_view_of_chair_back_36 = sum(hist_test.*hist_front)/(norm(hist_test)*norm(hist_front))
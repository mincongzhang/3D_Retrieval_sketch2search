clc
clear
close all

%%
% get model point cloud 
list = [2 5 6 8 9 12 14 16 18 26 36 37 57 64 69 76 77 82 87 88 102 124 128 140 142 148 164 168 188 195];
% list = [26]; 
for i = 1:length(list)
    close all
    filename = ['back_',num2str(list(i)),'_.mat'];
    hist_filename = ['back_',num2str(list(i)),'_hist_front.mat'];
    load(filename);

    X = data(:,1);
    Y = data(:,2);
    Z = data(:,3);
    
    %avoid negative & move to original coordinate
    X = X - min(X);
    Y = Y - min(Y);
    Z = Z - min(Z);
    
    MAX = max([max(X),max(Y),max(Z)]);
    X = X./MAX;
    Y = Y./MAX;
    Z = Z./MAX;

    x_floor = min([min(X(:)),min(Y(:)),min(Z(:))]);
    x_ceil  = max([max(X(:)),max(Y(:)),max(Z(:))]);
    
    figure,plot(X,Y,'.'); title('front');
    set(gca, 'XLim', [x_floor x_ceil]);
    set(gca, 'YLim', [x_floor x_ceil]);
    print(gcf,'-dpng',[num2str(list(i)),'.png'])   % 保存为png格式的图片
    %pause(1)
    figure,plot(Z,Y,'.'); title('side');
    set(gca, 'XLim', [x_floor x_ceil]);
    set(gca, 'YLim', [x_floor x_ceil]);
    %pause(1)
    figure,plot(X,Z,'.'); title('up');
    set(gca, 'XLim', [x_floor x_ceil]);
    set(gca, 'YLim', [x_floor x_ceil]);
    %pause(1)
    %%
    %map to 32x32 grid
    grid_front = zeros(32,32);
    grid_side  = zeros(32,32);
    grid_up    = zeros(32,32);
    X = round(X.*32);
    Y = round(Y.*32);
    Z = round(Z.*32);
    n_points = length(X);

    %frond Grid (to avoid double count sketch point in the same grid)
    for i = 1:n_points
        grid_front(Y(i)+1,X(i)+1) = 1; % avoid coordinate = 0, after normalize the sketch  actually the grid is 1 - 33
    end

    [Y_grid_front,X_grid_front] = find(grid_front == 1);
    figure,plot(X_grid_front,Y_grid_front,'.');title('grid front'); 
    set(gca, 'XLim', [1.0 33.0]); %actually the grid is 1 - 33
    set(gca, 'YLim', [1.0 33.0]);
    %pause(1)
    
    % %side Grid (to avoid double count sketch point in the same grid)
    % for i = 1:n_points
    %     grid_side(Y(i)+1,Z(i)+1) = 1; % avoid coordinate = 0, after normalize the sketch
    % end
    % [Y_grid_side,Z_grid_side] = find(grid_side == 1);
    % figure,plot(Z_grid_side,Y_grid_side,'*');title('grid side');
    % set(gca, 'XLim', [0.0 32.0]);
    % set(gca, 'YLim', [0.0 32.0]);
    % 
    % %up Grid (to avoid double count sketch point in the same grid)
    % for i = 1:n_points
    %     grid_up(Z(i)+1,Y(i)+1) = 1; % avoid coordinate = 0, after normalize the sketch
    % end
    % [Y_grid_up,Z_grid_up] = find(grid_up == 1);
    % figure,plot(Z_grid_up,Y_grid_up,'*');title('grid side');
    % set(gca, 'XLim', [0.0 32.0]);
    % set(gca, 'YLim', [0.0 32.0]);

    %%
    % get centroid in 32x32 grid
    x_centroid_front = mean(X_grid_front); 
    y_centroid_front = mean(Y_grid_front);
    % x_centroid_side  = mean(X_grid_side); 
    % y_centroid_side  = mean(Y_grid_side);
    % x_centroid_up    = mean(X_grid_up); 
    % y_centroid_up    = mean(Y_grid_up);

    %%
    %histogram
    grid_length = length(X_grid_front(:));
    for i = 1:grid_length
        diff_front(i) = sqrt((X_grid_front(i)-x_centroid_front)^2 + (Y_grid_front(i)-y_centroid_front)^2);
    end
    diff_front = diff_front./(32*sqrt(2));
    hist_front = zeros(ceil(100*sqrt(2)),1);

    for i = 1:grid_length
        hist_front(round(diff_front(i).*100)+1) = hist_front(round(diff_front(i).*100)+1) +1;
    end
    %hist_front = hist_front./max(hist_front);

    %figure,plot(hist_front); title('histogram')
    save(hist_filename,'hist_front')
end

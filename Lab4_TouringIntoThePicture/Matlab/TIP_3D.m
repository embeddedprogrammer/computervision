% sample code on how to display 3D sufraces in Matlab
figure(3);

% define a surface in 3D (need at least 6 points, for some reason)
point = @(x, y, z) struct('x', x, 'y', y, 'z', z);
% p1 = point(0, -1, 1);
% p2 = point(0, -1, 0);
% p3 = point(0,  1, 0);
% p4 = point(0,  1, 1);
% addSurface(bim, bim_alpha, p1, p2, p3, p4)
% hold on;
% 
% p1 = point(0, -1, 1);
% p2 = point(0, -1, 0);
% p3 = point(-2, -1, 0);
% p4 = point(-2, -1, 1);
% addSurface(bim, bim_alpha, p1, p2, p3, p4)
% 
% p1 = point(-2, -1, 1);
% p2 = point(-2, -1, 0);
% p3 = point(-2,  1, 0);
% p4 = point(-2,  1, 1);
% addSurface(bim, bim_alpha, p1, p2, p3, p4)

fileID = fopen('../world.txt','r');
numpollies = fscanf(fileID, 'NUMPOLLIES %d\n', 1)

hold on;
for i=1:numpollies
	corners = fscanf(fileID, 'SHAPE %d\n', 1);
	if corners == 4
		p = cell([1 4]);
		for j = 1:4
			coords = fscanf(fileID, '%f %f %f %f %f\n', 5);
			p{j} = point(-coords(3), coords(1), coords(2));
		end
		name = fscanf(fileID, '%s\n', 1);
		addSurface(bim, bim_alpha, p{1}, p{2}, p{3}, p{4});
	end
end
fclose(fileID);



campos([0 0 0]);
camdir = [1 0 0];
camtarget(campos + camdir);

% Some 3D magic...
axis equal;  % make X,Y,Z dimentions be equal
axis vis3d;  % freeze the scale for better rotations
axis off;    % turn off the stupid tick marks
camproj('perspective');  % make it a perspective projection
% use the "rotate 3D" button on the figure or do "View->Camera Toolbar"
% to rotate the figure
% or use functions campos and camtarget to set camera location 
% and viewpoint from within Matlab code

set(gcf, 'KeyPressFcn', @pressKeyCb);
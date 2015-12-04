function TIP_3D
	% sample code on how to display 3D sufraces in Matlab
	figure(3);

	% define a surface in 3D (need at least 6 points, for some reason)
	point = @(x, y, z) struct('x', x, 'y', y, 'z', z);

	fileID = fopen('world.txt','r');
	numpollies = fscanf(fileID, 'NUMPOLLIES %d\n', 1);

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
			im = imread(name);
			addSurface(im, p{1}, p{2}, p{3}, p{4});
		end
	end
	fclose(fileID);



	campos([0 0 0]);
	camdir = [1 0 0];
	camtarget(campos + camdir);
	camva(50)

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
end
function addSurface(bim, pg1, pg2, pg3, pg4)
	%Change of variables to convert C++ file ordering to matlab ordering.
	p1 = pg1;
	p2 = pg4;
	p3 = pg2;
	p4 = pg3;
	
	% define a surface in 3D (need at least 6 points, for some reason)
	planex = [p1.x  (p1.x + p2.x) / 2  p2.x; p3.x  (p3.x + p4.x) / 2  p4.x];
	planey = [p1.y  (p1.y + p2.y) / 2  p2.y; p3.y  (p3.y + p4.y) / 2  p4.y];
	planez = [p1.z  (p1.z + p2.z) / 2  p2.z; p3.z  (p3.z + p4.z) / 2  p4.z];

	% create the surface and texturemap it with a given image
	w = warp(planex,planey,planez,bim);
	
	% some alpha-channel magic to make things transparent
	bim_alpha = ones([size(bim,1) size(bim,2)]);
% 	for x = 1:size(bim,1)
% 		for y = 1:size(bim,2)
% 			if bim(x, y, 1) < 15 && bim(x, y, 2) < 15 && bim(x, y, 3) < 15
% 				bim_alpha(x, y) = 0;
% 			end
% 		end
% 	end
	bim_alpha(1, 1) = 0;
	alpha(w, 'texture');
	alpha(w, bim_alpha);
end
function pressKeyCb(hObject, callbackdata)
	if(strcmp(callbackdata.Key, 'leftarrow'))
% 		camdir = unit_vec(camtarget - campos);
% 		camdir(1:2) = rotate(camdir(1:2), 10*pi/180);
% 		camtarget(campos + camdir)
		camorbit(10, 0)
	elseif(strcmp(callbackdata.Key, 'rightarrow'))
		camdir = unit_vec(camtarget - campos);
		camdir(1:2) = rotate(camdir(1:2), -10*pi/180);
		camtarget(campos + camdir)
	elseif(strcmp(callbackdata.Key, 'uparrow'))
		camdir = unit_vec(camtarget - campos);
		campos(campos + camdir*.3)
		camtarget(campos + camdir)
	elseif(strcmp(callbackdata.Key, 'downarrow'))
		camdir = unit_vec(camtarget - campos);
		campos(campos - camdir*.3)
		camtarget(campos + camdir)
	else
		callbackdata.Key
	end
end
function u = unit_vec(u)
	u = u / norm(u);
end
function u = rotate(u, theta)
	u = u*[cos(theta) -sin(theta); sin(theta) cos(theta)];
end
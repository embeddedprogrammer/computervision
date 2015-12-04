function TIP_3D
	fileID = fopen('world.txt','r');
	numpollies = fscanf(fileID, 'NUMPOLLIES %d\n', 1);

	hold on;
	for i=1:numpollies
		corners = fscanf(fileID, 'SHAPE %d\n', 1);
		if corners == 4
			p = cell([1 4]);
			for j = 1:4
				temp = fscanf(fileID, '%f %f %f %f %f\n', 5);
				p{j} = temp(1:3);
			end
			p = nudgeCorners(p); 
			
			name = fscanf(fileID, '%s\n', 1);
			[im, ~, imAlpha]  = imread('result.png');
			if(isempty(imAlpha)) %If no alpha channel, create by setting black pixels to transparent.
				imAlpha = fudgeAlphaChannel(im);
			end			
			addSurface(im, imAlpha, p{1}, p{2}, p{3}, p{4});
		end
	end
	fclose(fileID);

	% Some 3D magic...
	campos([0 0 0]); %Set camera position and direction
	camdir = [1 0 0];
	camtarget(campos + camdir);
	camva(50);   % Set view andle
	axis equal;  % make X,Y,Z dimentions be equal
	axis vis3d;  % freeze the scale for better rotations
	axis off;    % turn off the stupid tick marks
	camproj('perspective');  % make it a perspective projection
	set(gcf, 'KeyPressFcn', @pressKeyCb);
end
function imAlpha = fudgeAlphaChannel(im)
	imAlpha = ones([size(im, 1) size(im, 2)]);
	for x = 1:size(im,1)
		for y = 1:size(im,2)
			if im(x, y, 1) < 15 && im(x, y, 2) < 15 && im(x, y, 3) < 15
				imAlpha(x, y) = 0;
			end
		end
	end
end
function p = nudgeCorners(p)
	%Expand corners to overlap by about 1% to reduce white lines between touching panels.
	v1 = unit_vec(p{2} - p{1}) * .01;
	v2 = unit_vec(p{4} - p{1}) * .01;
	p{1} = p{1} - v1 - v2;
	p{2} = p{2} + v1 - v2;
	p{3} = p{3} + v1 + v2;
	p{4} = p{4} - v1 + v2;
	pp = cell([1 4]);
	
	%Change of variables to convert world.txt coordinates to matlab coordinates.
	for j = 1:4
		pp{j} = point(-p{j}(3), p{j}(1), p{j}(2));
	end
	p{1} = pp{1};
	p{2} = pp{4};
	p{3} = pp{2};
	p{4} = pp{3};
end
function p = point(x, y, z)
	p = struct('x', x, 'y', y, 'z', z);
end
function addSurface(im, imAlpha, p1, p2, p3, p4)
	% define a surface in 3D (need at least 6 points, for some reason)
	planex = [p1.x  (p1.x + p2.x) / 2  p2.x; p3.x  (p3.x + p4.x) / 2  p4.x];
	planey = [p1.y  (p1.y + p2.y) / 2  p2.y; p3.y  (p3.y + p4.y) / 2  p4.y];
	planez = [p1.z  (p1.z + p2.z) / 2  p2.z; p3.z  (p3.z + p4.z) / 2  p4.z];

	% create the surface and texturemap it with a given image
	w = warp(planex,planey,planez,im);
	alpha(w, 'texture');
	alpha(w, imAlpha);
end
function pressKeyCb(~, callbackdata)
	if(strcmp(callbackdata.Key, 'leftarrow'))
		camorbit(10, 0)
	elseif(strcmp(callbackdata.Key, 'rightarrow'))
		camorbit(-10, 0)
	elseif(strcmp(callbackdata.Key, 'uparrow'))
		camdolly(0, 0, .3)
	elseif(strcmp(callbackdata.Key, 'downarrow'))
		camdolly(0, 0, -.3)
	end
end
function u = unit_vec(u)
	u = u / norm(u);
end
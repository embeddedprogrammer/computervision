function pressKeyCb(hObject, callbackdata)
% Character - The character that displays as a result of pressing the key or keys. The character can be empty or unprintable.
% Modifier  - A cell array containing the names of one or more modifier keys that are being pressed (such as , control, alt, shift). On Macintosh computers, the cell array contains 'command' when the command modifier key is pressed.
% Key	      - The key being pressed, identified by the (lowercase) label on the key, or a descriptive string.
% Source	  - The object that has focus when the user presses the key.
% Eventname - The action that caused the callback function to execute.
	if(strcmp(callbackdata.Key, 'leftarrow'))
		camdir = unit_vec(camtarget - campos);
		camdir(1:2) = rotate(camdir(1:2), 10*pi/180);
		camtarget(campos + camdir)
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
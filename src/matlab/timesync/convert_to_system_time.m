function outtimes = convert_to_system_time(timesync,sensor_name,rawtimes)
	% outtimes = convert_to_system_time(timesync, sensor_name, rawtimes)
	%
	%	Will use the time sync structure to convert raw sensor
	%	times to the common system clock.
	%
	% arguments:
	%
	%	timesync -	The time synchronizations structure read
	%			in from the time sync xml file
	%
	%	sensor_name -	The name of the sensor to synchronize
	%
	%	rawtimes -	A list of timestamps in the sensor's raw
	%			clock to synchronize
	%
	% output:
	%
	%	outtimes -	The sensor times, converted to the common
	%			clock.
	%

	% find the sensor in the timesync structure
	for s = [1:length(timesync)]

		% check if this is the correct sensor
		if(strcmp(timesync(s).name, sensor_name))

			% s is the correct sensor, apply the conversion
			outtimes = timesync(s).slope * rawtimes ...
					+ timesync(s).offset;
			return;
		end
	end

	% if got here, it means that no sensor matched the given name
	error(['Could not find sensor of name ', sensor_name]);
end

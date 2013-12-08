%
% process_bmp_data.m
% Nicholas Corso <ncorso@eecs.berkeley.edu>
%
% function [data] = process_bmp_data(data,header)
%
% This function processes the raw data from the BMP085 binary data file and
% converts it into real units.  In addition the approximate height from sea
% level is calculated.
%
% For information on how these formulas work see the BMP085 datasheet.
%
% Inputs :
%   data    -   This is the data structure that is returned by the
%               read_bmp_data.m function.
%   header  -   This is the header structure that contains all of the
%               required metadata for converting the barometer data into
%               real pressure units.  This is also returned by the
%               read_bmp_data function.
%
% Outputs :
%   data    -   The updated data structure.  The fields .temperature and
%               .pressure now contain real valued units.  In addition a the
%               .altitude field was added which is the approximate height 
%               from sea level
%        
function [data] = process_bmp_data(data,header)

% Read out the coeffs
AC1 = header.calib_coeffs(1);
AC2 = header.calib_coeffs(2);
AC3 = header.calib_coeffs(3);
AC4 = header.calib_coeffs(4);
AC5 = header.calib_coeffs(5);
AC6 = header.calib_coeffs(6);
B1 = header.calib_coeffs(7);
B2 = header.calib_coeffs(8);
MB = header.calib_coeffs(9);
MC = header.calib_coeffs(10);
MD = header.calib_coeffs(11);

data.altitude = zeros(size(data.pressure));

for i = 1:1:header.num_readings
    
    % Calculate the true temp
    X1 = floor((data.temperature(i)-AC6)*AC5/2^15);
    X2 = floor(MC*2^11/(X1+MD));
    B5 = floor(X1+X2);
    data.temperature(i) = floor((B5+8)/2^4);
    
    % Calculate true temperature
    B6 = floor(B5-4000);
    X1 = floor((B2*(B6*B6/2^12))/2^11);
    X2 = floor(AC2*B6/2^11);
    X3 = floor(X1+X2);
    B3 = floor(((AC1*4+X3)*2^(header.oversampling) + 2)/4);
    X1 = floor(AC3*B6/2^13);
    X2 = floor((B1*(B6*B6/2^12))/2^16);
    X3 = floor(((X1+X2)+2)/2^2);
    B4 = floor(AC4*(X3+32768)/2^15);
    B7 = floor((data.pressure(i) - B3)*(50000/2^(header.oversampling)));
    p = floor((B7*2)/B4);
    X1 = floor(p/2^8)*floor(p/2^8);
    X1 = floor((X1*3038)/2^16);
    X2 = floor((-7357*p)/2^16);
    data.pressure(i) = floor(p+(X1+X2+3791)/2^4);
    
    % Calculate the Height
    data.altitude(i) = 44330*(1-(data.pressure(i)/100/1013.25)^(1/5.255));
    
end


end
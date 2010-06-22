//
// Fan Control
// Copyright 2006 Lobotomo Software 
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//

#import "MFDaemon.h"
#import "MFProtocol.h"
#import "MFDefinitions.h"
#import "smc.h"

#define MFApplicationIdentifier     "com.lobotomo.MacProFan"


@implementation MFDaemon

- (id)init
{
    if (self = [super init]) {
        // set sane defaults
        lowerThreshold = 50.0;
        upperThreshold = 80.0;
        baseRpm = 1500;
        maxRpm = MFMaxRpm;
		SMCOpen();
		SMCVal_t val;		
		strcpy(val.key, "FS!\0");
		val.bytes[0] = 0x00;
		val.bytes[1] = 0x03;
		val.dataSize = 2;
		SMCWriteKey(val);
		SMCClose();
    }
    return self;
}

// store preferences
- (void)storePreferences
{
    CFPreferencesSetValue(CFSTR("baseRpm"), (CFPropertyListRef)[NSNumber numberWithInt:baseRpm],
                          CFSTR(MFApplicationIdentifier), kCFPreferencesAnyUser, kCFPreferencesCurrentHost);
    CFPreferencesSetValue(CFSTR("lowerThreshold"), (CFPropertyListRef)[NSNumber numberWithFloat:lowerThreshold],
                          CFSTR(MFApplicationIdentifier), kCFPreferencesAnyUser, kCFPreferencesCurrentHost);
    CFPreferencesSetValue(CFSTR("upperThreshold"), (CFPropertyListRef)[NSNumber numberWithFloat:upperThreshold],
                          CFSTR(MFApplicationIdentifier), kCFPreferencesAnyUser, kCFPreferencesCurrentHost);
    CFPreferencesSetValue(CFSTR("fahrenheit"), (CFPropertyListRef)[NSNumber numberWithBool:fahrenheit],
                          CFSTR(MFApplicationIdentifier), kCFPreferencesAnyUser, kCFPreferencesCurrentHost);
    CFPreferencesSynchronize(CFSTR(MFApplicationIdentifier), kCFPreferencesAnyUser, kCFPreferencesCurrentHost);
}

// read preferences
- (void)readPreferences
{
    CFPropertyListRef property;
    property = CFPreferencesCopyValue(CFSTR("baseRpm"), CFSTR(MFApplicationIdentifier),
               kCFPreferencesAnyUser, kCFPreferencesCurrentHost);
    if (property) {
        baseRpm = [(NSNumber *)property intValue];
    }
    property = CFPreferencesCopyValue(CFSTR("lowerThreshold"), CFSTR(MFApplicationIdentifier),
               kCFPreferencesAnyUser, kCFPreferencesCurrentHost);
    if (property) {
        lowerThreshold = [(NSNumber *)property floatValue];
    }
    property = CFPreferencesCopyValue(CFSTR("upperThreshold"), CFSTR(MFApplicationIdentifier),
               kCFPreferencesAnyUser, kCFPreferencesCurrentHost);
    if (property) {
        upperThreshold = [(NSNumber *)property floatValue];
    }
    property = CFPreferencesCopyValue(CFSTR("fahrenheit"), CFSTR(MFApplicationIdentifier),
               kCFPreferencesAnyUser, kCFPreferencesCurrentHost);
    if (property) {
        fahrenheit = [(NSNumber *)property boolValue];
    }
}

// this gets called after application start
- (void)start
{
    [self readPreferences];
    [NSTimer scheduledTimerWithTimeInterval:0.5 target:self selector:@selector(timer:) userInfo:nil repeats:YES];
}

// control loop called by NSTimer
- (void)timer:(NSTimer *)aTimer
{
    double temp;
    int targetRpm;
    int step;

    SMCOpen();

    temp = SMCGetTemperature(SMC_KEY_CPU_TEMP);
    
    if (temp < lowerThreshold) {
        targetRpm = baseRpm;
    } else if (temp > upperThreshold) {
        targetRpm = maxRpm;
    } else {
        targetRpm = baseRpm + (temp - lowerThreshold) / (upperThreshold - lowerThreshold) * (maxRpm - baseRpm);
    }

    // adjust fan speed in reasonable steps - no need to be too dynamic
    /*if (currentRpm == 0) {
        step = targetRpm;
    } else {
        step = (targetRpm - currentRpm) / 6;
        if (abs(step) < 20) {
            step = 0;
        }
    }
    targetRpm = currentRpm = currentRpm + step;*/
    
    SMCSetFanRpm(SMC_KEY_FAN0_RPM_MIN, targetRpm);
    SMCSetFanRpm(SMC_KEY_FAN1_RPM_MIN, targetRpm);

    SMCClose();

    // save preferences
    if (needWrite) {
        [self storePreferences];
        needWrite = NO;
    }
}

// accessors
- (int)baseRpm
{
	return baseRpm;
}

- (void)setBaseRpm:(int)newBaseRpm
{
	baseRpm = newBaseRpm;
    needWrite = YES;
}

- (float)lowerThreshold
{
	return lowerThreshold;
}

- (void)setLowerThreshold:(float)newLowerThreshold
{
	lowerThreshold = newLowerThreshold;
    needWrite = YES;
}

- (float)upperThreshold
{
	return upperThreshold;
}

- (void)setUpperThreshold:(float)newUpperThreshold
{
	upperThreshold = newUpperThreshold;
    needWrite = YES;
}

- (BOOL)fahrenheit
{
	return fahrenheit;
}

- (void)setFahrenheit:(BOOL)newFahrenheit
{
	fahrenheit = newFahrenheit;
    needWrite = YES;
}

- (void)temperature:(float *)temperature leftFanRpm:(int *)leftFanRpm rightFanRpm:(int *)rightFanRpm
{
    SMCOpen();
    if (temperature) {
        *temperature = SMCGetTemperature(SMC_KEY_CPU_TEMP);
    }
    if (leftFanRpm) {
        *leftFanRpm = SMCGetFanRpm(SMC_KEY_FAN0_RPM_CUR);
    }
    if (rightFanRpm) {
        *rightFanRpm = SMCGetFanRpm(SMC_KEY_FAN1_RPM_CUR);
    }
    SMCClose();
}

@end

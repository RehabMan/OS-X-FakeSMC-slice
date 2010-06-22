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

#import "MFPreferencePane.h"
#import "MFProtocol.h"
#import "MFTemperatureTransformer.h"
#import "MFChartView.h"


@implementation MFPreferencePane


- (id)initWithBundle:(NSBundle *)bundle
{
    if (self = [super initWithBundle:bundle]) {
        transformer = [MFTemperatureTransformer new];
        [NSValueTransformer setValueTransformer:transformer forName:@"MFTemperatureTransformer"];
    }
    return self;
}

- (void)dealloc
{
    [transformer release];
    [super dealloc];
}

- (void)updateOutput:(NSTimer *)aTimer
{
    float temp;
    int leftFanRpm;
    int rightFanRpm;

    [daemon temperature:&temp leftFanRpm:&leftFanRpm rightFanRpm:&rightFanRpm];
    [leftFanField setIntValue:leftFanRpm];
    [rightFanField setIntValue:rightFanRpm];
    [temperatureField setStringValue:[transformer transformedValue:[NSNumber numberWithFloat:temp]]];
    [chartView setCurrentTemp:temp];
}

- (void)awakeFromNib
{
    // connect to daemon
    NSConnection *connection = [NSConnection connectionWithRegisteredName:MFDaemonRegisteredName host:nil];
    daemon = [[connection rootProxy] retain];
    [(id)daemon setProtocolForProxy:@protocol(MFProtocol)];

    // set transformer mode
    [transformer setFahrenheit:[self fahrenheit]];

    // connect to object controller
    [fileOwnerController setContent:self];
}

// sent before preference pane is displayed
- (void)willSelect
{
    // update chart
    [chartView setBaseRpm:[self baseRpm]];
    [chartView setLowerThreshold:[self lowerThreshold]];
    [chartView setUpperThreshold:[self upperThreshold]];

    // update output immediatly, then every 5 seconds
    [self updateOutput:nil];
    timer = [NSTimer scheduledTimerWithTimeInterval:5.0 target:self selector:@selector(updateOutput:)
                     userInfo:nil repeats:YES];
}

// sent after preference pane is ordered out
- (void)didUnselect
{
    // stop updates
    [timer invalidate];
    timer = nil;
}

// accessors (via daemon)
- (int)baseRpm
{
	return [daemon baseRpm];
}

- (void)setBaseRpm:(int)newBaseRpm
{
	[daemon setBaseRpm:newBaseRpm];
	[chartView setBaseRpm:newBaseRpm];
}

- (float)lowerThreshold
{
	return [daemon lowerThreshold];
}

- (void)setLowerThreshold:(float)newLowerThreshold
{
	[daemon setLowerThreshold:newLowerThreshold];
	[chartView setLowerThreshold:newLowerThreshold];
}

- (float)upperThreshold
{
	return [daemon upperThreshold];
}

- (void)setUpperThreshold:(float)newUpperThreshold
{
	[daemon setUpperThreshold:newUpperThreshold];
	[chartView setUpperThreshold:newUpperThreshold];
}

- (BOOL)fahrenheit
{
	return [daemon fahrenheit];
}

- (void)setFahrenheit:(BOOL)newFahrenheit
{
	[daemon setFahrenheit:newFahrenheit];
    [transformer setFahrenheit:newFahrenheit];
    // force display update
    [self updateOutput:nil];
    [fileOwnerController setContent:nil];
    [fileOwnerController setContent:self];
}

@end

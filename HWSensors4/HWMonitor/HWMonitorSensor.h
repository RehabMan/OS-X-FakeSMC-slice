//
//  NSSensor.h
//  HWSensors
//
//  Created by mozo,Navi on 22.10.11.
//  Copyright (c) 2011 mozo. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "ISPSmartController.h"

enum {
    TemperatureSensorGroup =   1,
    VoltageSensorGroup =       2,
    TachometerSensorGroup =    3,
    FrequencySensorGroup =     4,
    MultiplierSensorGroup =    5,
    HDSmartTempSensorGroup = 6, 
    BatterySensorsGroup = 7,

};
typedef NSUInteger SensorGroup;

@interface HWMonitorSensor : NSObject

@property (readwrite, retain) NSString *    key;
@property (readwrite, retain) NSString *    type;
@property (readwrite, assign) SensorGroup   group;
@property (readwrite, retain) NSString *    caption;
@property (readwrite, retain) id            object;
@property (readwrite, assign) BOOL          favorite;



+ (unsigned int)        swapBytes:(unsigned int) value;

//+ (NSDictionary *)      populateValues;
//+ (NSData *)            populateValueForKey:(NSString *)key;
+ (NSData *)            readValueForKey:(NSString *)key;

- (HWMonitorSensor *)   initWithKey:(NSString *)aKey andType: aType andGroup:(NSUInteger)aGroup withCaption:(NSString *)aCaption;
- (NSString *)          formatedValue:(NSData *)value;

@end

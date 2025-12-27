 /*
 *  Created on: Feb 25, 2025
 *      Author: ADMIN-HPZ2
 */

#ifndef MAIN_PROGRAM_AUTO_AUTO_H_
#define MAIN_PROGRAM_AUTO_AUTO_H_
#ifdef __cplusplus
extern "C" {
#endif

#define SPEED_LEVEL_0 0    // m/s
#define SPEED_LEVEL_1 0.005
#define SPEED_LEVEL_2 0.05
#define SPEED_LEVEL_3 0.35
#define SPEED_LEVEL_4 0.6

void autoTaskInit();
void autoTaskSupend();
void autoTaskResume();

#ifdef __cplusplus
}
#endif
#endif /* MAIN_PROGRAM_AUTO_AUTO_H_ */

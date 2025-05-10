/*
 * manual.h
 *
 *  Created on: Feb 20, 2025
 *      Author: ADMIN-HPZ2
 */

#ifndef MAIN_PROGRAM_MANUAL_MANUAL_H_
#define MAIN_PROGRAM_MANUAL_MANUAL_H_
#ifdef __cplusplus
extern "C" {
#endif

#define MANUAL_ACC 0.25  // m/s²
#define MANUAL_DEC 0.3   // m/s²

void manualTaskInit();
void manualTaskSupend();
void manualTaskResume();

#ifdef __cplusplus
}
#endif
#endif /* MAIN_PROGRAM_MANUAL_MANUAL_H_ */

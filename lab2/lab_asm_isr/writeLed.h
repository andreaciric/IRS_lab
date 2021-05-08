/**
 * @file writeLed.h
 * @brief Decleration of function used to write (0-9) to 7seg display
 *
 *
 * @date 06.05.2021.
 * @author Andrea Ciric (andreaciric23@gmail.com)
 *
 * @version [1.0 - 04/2021] Initial version
 *
 **/

#ifndef FUNCTION_H_
#define FUNCTION_H_

/**
 * @brief Function used to write to 7seg display
 * @param digit - value 0-9 to be displayed
 *
 * Function writes data a-g on PORT6.
 * It is assumed that appropriate 7seg display is enabled.
 */
extern void WriteLed(unsigned int digit);

#endif /* FUNCTION_H_ */

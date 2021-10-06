#include<stdio.h>
#include <stdlib.h> 
#include <ctype.h>
#include <math.h>
#include <string.h>
#include<locale.h>

int parse_and_calculate(char** temp, double* ans, int* brackets);
int getNumeric(char** ptr, double* rsf);

int fnCall;
#define MAXCALLS 10000

int checkRussian(char* str)
{
	int i;
	int len = strlen(str);
	for (i = 0; i < len; i++)
		if (str[i] < 0)
			return 1;
	return 0;
}
// double variableValues[60];
// int variableUsage [60];
// //resets all variables
// void resetVar()
// {
// 	for (int i = 0; i < 60; i++)
// 	{
// 		variableUsage[i] = 0;
// 		variableValues[i] = 0;
// 	}
// }

// reads a char pointed to by pointer and move pointer if necessary
// returns next non-whitespace char
char readChar(char** ptr)
{
	while (isspace((unsigned char)(**ptr)) && **ptr != '\0')
		(*ptr)++;
	return **ptr;
}

// reads a char pointed to by the pointer 
// returns next char, even if it's whitespace
char readTrueChar(char** ptr)
{
	return **ptr;
}

void printResult(FILE* output, char* expression, double result, int code, int sof)
{
	switch (code)
	{
	case -1:
		fprintf(output, "%s", expression); //newline or comment
		break;
	case 0:
		fprintf(output, "%s == %G", expression, result);
		break;
	case 1:
		fprintf(output, "%s == ERROR: Syntax Error", expression); // unexpected character while parsing
		break;
	case 2:
		fprintf(output, "%s == ERROR: brackets", expression); // number of '(' and ')' differ
		break;
	case 3:
		fprintf(output, "%s == ERROR: SQRT negative", expression); // attempted sqrt of negative number
		break;
	case 4:
		fprintf(output, "%s == ERROR: Divide by zero", expression); // attempted division by zero
		break;
	case 5:
		fprintf(output, "%s == ERROR: asin input out of range", expression); // attempted asin of >1 || <-1
		break;
	case 6:
		fprintf(output, "%s == ERROR: acos input out of range", expression); // attempted acos of >1 || <-1
		break;
	case 7:
		fprintf(output, "%s == ERROR: ln input out of range", expression); // attempted ln of <= 0
		break;
	case 8:
		fprintf(output, "%s == ERROR: invalid cot input", expression); // attempted cot on a multiple of pi
		break;
	case 9:
		fprintf(output, "%s == ERROR: invalid exponent combination", expression); // x^y returned NAN or inf
		break;
	case 10:
		fprintf(output, "ERROR: memory allocation error"); //realloc or malloc failed
		break;
	case 11:
		fprintf(output, "%s == ERROR: invalid log inputs", expression); // log resulted in inf or nan
		break;
	case 12:
		fprintf(output, "%s == ERROR: Russian character detected", expression); // Russian
		break;
	case 13:
		fprintf(output, "%s == ERROR: Stack overflow", expression); // stack overflow
		break;
	default:
		fprintf(output, "%s == ERROR: Unknown", expression);

	}
	if (sof)
		fprintf(output, "\n");
}

// The calculator of the processor, this part does the simple math
// answer is stored in pointer ans, returns 0 on success or errorCode
int ALU(double* ans, double operand1, int operator, double operand2)
{
	switch (operator)
	{
	case 1:
		*ans = operand1 + operand2;
		break;
	case 2:
		*ans = operand1 - operand2;
		break;
	case 3:
		*ans = operand1 * operand2;
		break;
	case 4:
		if (operand2 == 0)
			return 4;
		*ans = operand1 / operand2;
		break;
	}
	return 0;
}

// starts parsing from ptr, determine the function and assign code
// returns 0 on success, errorCode otherwise
// sqrt, sin, cos, tg, ctg, arcsin, arccos, arctg, ln, floor, ceil, log
int parseFunction(char** ptr, int* code)
{
	char* fn = "\0";
	//int errorCode = 0;
	char a = **ptr;
	switch (a)
	{
	case 's':
		(*ptr)++;
		if (readTrueChar(ptr) == 'q')
		{
			fn = "rt";
			*code = 1;
		}
		else if (readTrueChar(ptr) == 'i')
		{
			fn = "n";
			*code = 2;
		}
		else
			return 1;
		break;
	case 'c':
		(*ptr)++;
		if (readTrueChar(ptr) == 'o')
		{
			fn = "s";
			*code = 3;
		}
		else if (readTrueChar(ptr) == 't')
		{
			fn = "g";
			*code = 5;
		}
		else if (readTrueChar(ptr) == 'e')
		{
			fn = "il";
			*code = 11;
		}
		else
			return 1;
		break;
	case 't':
		fn = "g";
		*code = 4;
		break;
	case 'a':
		(*ptr)++;
		if (readTrueChar(ptr) != 'r')
			return 1;
		(*ptr)++;
		if (readTrueChar(ptr) != 'c')
			return 1;
		(*ptr)++;
		if (readTrueChar(ptr) == 's')
		{
			fn = "in";
			*code = 6;
		}
		else if (readTrueChar(ptr) == 'c')
		{
			fn = "os";
			*code = 7;
		}
		else if (readTrueChar(ptr) == 't')
		{
			fn = "g";
			*code = 8;
		}
		else
			return 1;
		break;
	case 'l':
		(*ptr)++;
		if (readTrueChar(ptr) == 'n')
		{
			fn = "";
			*code = 9;
		}
		else if (readTrueChar(ptr) == 'o')
		{
			fn = "g";
			*code = 12;
		}
		else
			return 1;
		break;
	case 'f':
		fn = "loor";
		*code = 10;
		break;
	default:
		return 1;
	}
	(*ptr)++;

	while (*fn != '\0')
	{
		if (*fn != readTrueChar(ptr) || readTrueChar(ptr) == '\0')
			return 1;
		(*ptr)++;
		fn++;
	}
	return 0;
}

// obtains the result of a mathematic function call
// returns 0 on success, errorCode otherwise.
int getFunction(char** ptr, double* operand, int* brackets)
{
	fnCall++;
	if (fnCall >= MAXCALLS)
		return 13;
	int functionCode, errorCode;
	double logTemp, term = 0, positive = 1;
	errorCode = parseFunction(ptr, &functionCode);
	if (errorCode)
		return errorCode;
	while (readChar(ptr) == '-')
	{
		positive *= -1;
		(*ptr)++;
	}
	if (functionCode == 12) // log, special treatment
	{
		if (readChar(ptr) == '(')
		{
			(*ptr)++;
			(*brackets)++;
			errorCode = parse_and_calculate(ptr, &logTemp, brackets);
			if (!errorCode)
				return 11;
			if (errorCode != -2)
				return errorCode;
			errorCode = parse_and_calculate(ptr, &term, brackets);
			if (errorCode)
				return errorCode;
			*operand = log(term) / log(logTemp);
			if (isinf(*operand) || isnan(*operand) || logTemp == 0)
				return 11;
			fnCall--;
			return 0;

		}
	}
	else if (readChar(ptr) == '(')
	{
		(*ptr)++;
		(*brackets)++;
		errorCode = parse_and_calculate(ptr, &term, brackets);
		if (errorCode)
			return errorCode;
	}
	else if (isdigit(readChar(ptr)))
	{
		errorCode = getNumeric(ptr, &term);
		if (errorCode)
			return errorCode;
	}
	else if (readChar(ptr) == 'e') //check constant
	{
		(*ptr)++;
		term = exp(1);
	}
	else if (readChar(ptr) == 'p')
	{
		(*ptr)++;
		if (readTrueChar(ptr) == 'i')
			term = acos(-1);
		else
			return 1;
		(*ptr)++;
	}
	else
	{
		errorCode = getFunction(ptr, &term, brackets);
		if (errorCode)
			return errorCode;
	}
	term *= positive;
	switch (functionCode)
	{
	case 1:
		if (term < 0)
			return 3;
		*operand = sqrt(term);
		break;
	case 2:
		*operand = sin(term);
		break;
	case 3:
		*operand = cos(term);
		break;
	case 4:
		*operand = tan(term);
		break;
	case 5:
		if (term == 0) //cot
			return 8;
		*operand = 1 / tan(term);
		break;
	case 6:
		if (term > 1 || term < -1)
			return 5;
		*operand = asin(term);
		break;
	case 7:
		if (term > 1 || term < -1)
			return 6;
		*operand = acos(term);
		break;
	case 8:
		*operand = atan(term);
		break;
	case 9:
		if (term <= 0)
			return 7;
		*operand = log(term);
		break;
	case 10:
		*operand = floor(term);
		break;
	case 11:
		*operand = ceil(term);
		break;
		// case 12:
		// 	break;
	default:
		return 1;
	}
	fnCall--;
	return 0;
}

// obtain a double from the string starting at *ptr
// reads until a non-numeric, non-decimal char is reached.
// returns 0 if success, 1 if two decimals encountered
int getNumeric(char** ptr, double* rsf)
{
	fnCall++;
	if (fnCall >= MAXCALLS)
		return 13;
	int dec = 0, exp = 0;
	if (readChar(ptr) == '-')
		while (readChar(ptr) == '-')
			(*ptr)++;
	*rsf = atof(*ptr);
	while (isdigit(**ptr) || **ptr == '.' || **ptr == 'e' || **ptr == 'E')
	{
		if (**ptr == '.')
		{
			if (dec == 1)
				return 1;
			else
				dec = 1;
		}
		if (**ptr == 'e' || **ptr == 'E')
		{
			if (exp == 1)
				return 1;
			else
			{
				if (*((*ptr) + 1) == '+' || *((*ptr) + 1) == '-')
				{
					(*ptr)++;
					if (*((*ptr) + 1) == ' ' || *((*ptr) + 1) == '-' || *((*ptr) + 1) == '+')
						return 1;
				}
				exp = 1;
			}
		}

		(*ptr)++;
	}
	fnCall--;
	return 0;
}

// grabs the operator char pointed to by *ptr and increments *ptr
// returns 0 if success, 1 if not an operation character.
int getOperator(char** ptr, int* operator, int* brackets)
{
	switch (readChar(ptr))
	{
	case '\0':
		*operator = 0;
		break;
	case '+':
		*operator = 1;
		break;
	case '-':
		*operator = 2;
		break;
	case '*':
		*operator = 3;
		break;
	case '/':
		*operator = 4;
		break;
	case ')':
		*operator = -1;
		break;
	case ',':
		*operator = 109; // 109 looks like log :)
		break;
		// case ';':
		// 	*operator = 60; // for variables
		// 	break;
	default:
		return 1;
	}
	if (*operator != 0 || *brackets == 0) // if missing close brackets
		(*ptr)++;
	return 0;
}

// get next operand and move pointer temp
// return 0 if success, errorCode otherwise
int getTerm(char** ptr, double* operand, int* brackets)
{
	fnCall++;
	if (fnCall >= MAXCALLS)
		return 13;
	// determine whether operand will be positive
	int positive = 1, errorCode = 0;
	while (readChar(ptr) == '-')
	{
		positive++;
		(*ptr)++;
	}
	positive = positive % 2;
	if (!positive)
		positive = -1;

	char curr = readChar(ptr);
	// (*ptr)++;
	// char nextCurr = readChar(ptr);
	// (*ptr)--;

	if (isdigit(curr)) // this term is a numeric
	{
		errorCode = getNumeric(ptr, operand);
		if (errorCode)
			return errorCode;
	}

	else if (curr == 'e' || curr == 'p') // this term is a constant
	{
		if (curr == 'e')
		{
			*operand = exp(1);
			(*ptr)++;
		}
		else
		{
			(*ptr)++;
			if (readTrueChar(ptr) == 'i')
			{
				*operand = acos(-1);
				(*ptr)++;
			}
			else
				return 1;
		}
	}
	else if (curr == '(') // this term is an expression
	{
		double temp;
		(*ptr)++;
		(*brackets)++;
		errorCode = parse_and_calculate(ptr, operand, brackets);
		if (errorCode)
			return errorCode;
		if (readChar(ptr) == 'e' || readChar(ptr) == 'E')
		{
			(*ptr)++;
			if (readChar(ptr) == '+')
				(*ptr)++;
			errorCode = getNumeric(ptr, &temp);
			if (errorCode)
				return errorCode;
			*operand = *operand * pow(10, temp);
		}
	}
	// else if (isalpha(curr) && !isalpha(nextCurr)) // variable stuff
	// {
	// 	if (nextCurr == '=') // assignment
	// 	{ 
	// 		double val;
	// 		(*ptr) += 2;
	// 		errorCode = parse_and_calculate(ptr, &val, brackets);
	// 		if (!errorCode)
	// 			return 1;
	// 		if (errorCode != -3)
	// 			return errorCode;
	// 		variableValues[curr - 65] = val;
	// 		variableUsage[curr - 65] = 1;
	// 	}
	// 	else //using
	// 	{
	// 		if (variableUsage[curr-65])
	// 			*operand = variableValues[curr-65];
	// 		else 
	// 			return 12;
	// 	}
	// }
	else // this term is (possibly) a function
	{
		// sqrt, sin, cos, tg, ctg, arcsin, arccos, arctg, ln, floor, ceil, log
		errorCode = getFunction(ptr, operand, brackets);
		if (errorCode)
			return errorCode;

	}
	if (readChar(ptr) == '^') //exponents, take care of this now
	{
		double temp;
		if (readChar(ptr) == '^')
		{
			(*ptr)++;
			errorCode = getTerm(ptr, &temp, brackets);
			if (errorCode)
				return errorCode;
			*operand = pow(*operand, temp);
			if (isnan(*operand) || isinf(*operand))
				return 9;
		}
	}
	*operand *= positive;
	fnCall--;
	return 0;
}

// calculate from pointer "expression" until terminator or ')' is reached.
// returns 0 if success, error code otherwise
int parse_and_calculate(char** temp, double* ans, int* brackets)
{
	fnCall++;
	if (fnCall >= MAXCALLS)
		return 13;
	int operator, nextOperator, errorCode;
	double operand1, operand2, operand3;
	errorCode = getTerm(temp, &operand1, brackets);
	if (errorCode)
		return errorCode;
	errorCode = getOperator(temp, &operator, brackets);
	if (errorCode)
		return errorCode;
	if (operator == 0) // NOP, return operand
	{
		*ans = operand1;
		fnCall--;
		return 0;
	}
	if (operator == -1) //close bracket
	{
		*ans = operand1;
		(*brackets)--;
		fnCall--;
		return 0;
	}
	if (operator == 109) // comma from log
	{
		*ans = operand1;
		return -2;
	}
	// if (operator == 60)
	// {
	// 	*ans = operand1;
	// 	return -3;
	// }

	errorCode = getTerm(temp, &operand2, brackets);
	if (errorCode)
		return errorCode;
	errorCode = getOperator(temp, &nextOperator, brackets);
	if (errorCode)
		return errorCode;

	while (1)
	{
		if (!nextOperator || nextOperator == -1 || nextOperator == 109) // if NOP, close bracket or comma
		{
			errorCode = ALU(ans, operand1, operator, operand2);
			if (errorCode)
				return errorCode;
			if (nextOperator == -1)
				(*brackets)--;
			if (nextOperator == 109)
				return -2;
			// if (nextOperator == 60)
			// 	return -3;
			fnCall--;
			return 0;
		}
		if (operator >= 3 || nextOperator <= 2)
		{
			errorCode = ALU(&operand1, operand1, operator, operand2);
			if (errorCode)
				return errorCode;
			operator = nextOperator;
			errorCode = getTerm(temp, &operand2, brackets);
			if (errorCode)
				return errorCode;
			errorCode = getOperator(temp, &nextOperator, brackets);
			if (errorCode)
				return errorCode;
		}
		else //execute order of operations
		{
			errorCode = getTerm(temp, &operand3, brackets);
			if (errorCode)
				return errorCode;
			errorCode = ALU(&operand2, operand2, nextOperator, operand3);
			if (errorCode)
				return errorCode;
			errorCode = getOperator(temp, &nextOperator, brackets);
			if (errorCode)
				return errorCode;
		}
	}


}


void LineCheck(char* expression, FILE* output, int sof)
{
	int emptystring = 0;
	int thecomment = 0;
	fnCall = 0;
	//int endfile = 0;
	int i = 0;

	char* temp = expression;

	if (readChar(&temp) == '/')
	{
		temp++;
		if (readTrueChar(&temp) == '/')
			thecomment = 1;
	}

	for (i = 0; i < (int)strlen(expression); i++)
	{
		if (!isspace((unsigned char)expression[i]))
		{
			emptystring = 1;
		}
	}

	if (thecomment == 1 || emptystring == 0)
	{
		printResult(output, expression, 0, -1, sof);
		return;
	}

	int brackets = 0, errorCode = 0;
	double ans;
	while (*temp != '\0')
	{
		if (checkRussian(temp))
		{
			printResult(output, expression, 0, 12, sof);
			return;
		}
		temp++;
	}
	temp = expression;

	errorCode = parse_and_calculate(&temp, &ans, &brackets);

	if (errorCode == -2) // if found outside of log. eCode = 1
		errorCode = 1;
	if (errorCode)
		printResult(output, expression, ans, errorCode, sof);
	else if (brackets != 0)
		printResult(output, expression, ans, 2, sof);
	else
		printResult(output, expression, ans, errorCode, sof);

}

int Computing(FILE* input, FILE* output)
{
	int c;
	char* expression;
	int counter = 0;
	int sof = 1;
	//long int pos = 0;

	expression = malloc(sizeof(char));
	if (expression == NULL)
	{
		printResult(output, expression, 0, 10, sof); //malloc failed
	}

	int allocFail = 0;

	while (1)
	{
		expression = realloc(expression, ((counter + 1) * sizeof(char)));
		c = getc(input);
		if (expression == NULL || allocFail)
		{
			counter = 0;
			allocFail = 1;
			if (c == '\n' || c == EOF || c == '\x1a')
			{
				if (c == EOF)
					sof = 0;
				printResult(output, expression, 0, 10, sof);
				allocFail = 0;
			}
		}
		else
		{
			expression[counter] = (char)c;
			counter++;
			if (c == '\n' || c == EOF || c == '\x1a') {
				if (c == EOF)
					sof = 0;
				expression[counter - 1] = '\0';
				LineCheck(expression, output, sof);
				counter = 0;
			}
		}
		if (c == EOF)
			break;

	}
	free(expression);
	return 0;
}

int main(int argc, char* argv[]) {
	FILE* Input = stdin;
	FILE* Output = stdout;
	setlocale(LC_CTYPE, "Russia");
	if (argc == 2) {
		fopen_s(&Input, argv[1], "r");
		if (Input == NULL) {
			fprintf(stdout, "%s", "ERROR: file is not existing");
			return 1;
		}
	}
	else if (argc == 3) {
		fopen_s(&Input, argv[1], "r");
		fopen_s(&Output, argv[2], "w");

		if (Input == NULL) {
			fprintf(stdout, "%s", "ERROR: file not existing");
			return 1;
		}
	}
	else if (argc > 3) {
		fprintf(stdout, "%s", "ERROR: you have more parameter than needed");
		return 1;
	}
	// resetVar();
	Computing(Input, Output);


	fclose(Input);
	fclose(Output);


	return 0;
}
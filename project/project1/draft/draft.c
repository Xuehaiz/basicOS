			/* Display each command */
			if (command == "ls") {
				listDir();
			}
			else if (command == "pwd") {
				showCurrentDir(); 
			}
			else if (command == "mkdir") {
				arg1 = strtok_r(line_cp, delimiter, &line_cp);
			}
			else if (command == "cd") {
				arg1 = strtok_r(line_cp, delimiter, &line_cp);
			}
			else if (command == "cp") {
				arg1 = strtok_r(line_cp, delimiter, &line_cp);
				arg2 = strtok_r(line_cp, delimiter, &line_cp);
			}
			else if (command == "mv") {
				arg1 = strtok_r(line_cp, delimiter, &line_cp);
				arg2 = strtok_r(line_cp, delimiter, &line_cp);
			}
			else if (command == "rm") {
				arg1 = strtok_r(line_cp, delimiter, &line_cp);
			}
			else if (command == "cat") {
				arg1 = strtok_r(line_cp, delimiter, &line_cp);
			}
			else {
				printf("Error! Unrecognized command: \n");
			}

			while (command != NULL) {
				
				printf("T%d: %s\n", i, command);
				i++;
				command = strtok_r(NULL, delimiter, &line_cp);
			}




					while (token != NULL) {
					//printf("line_cp: %s\n", line_cp);
					token = strtok_r(line_cp, delimiter, &line_cp);
					if (token == ";") {
						printf("yes, i did come here\n");
						goto start_over;
					}
				}


				if (strcmp(command, "ls") != 0 || strcmp(command, "pwd") != 0 || strcmp(command, "mkdir") != 0
					|| strcmp(command, "cd") != 0 || strcmp(command, "cp") != 0 || strcmp(command, "mv") != 0
					|| strcmp(command, "rm") != 0 || strcmp(command, "cat") != 0) {
					printf("Error! Unrecognized command: %s\n", command);
				}




			//printf("command: %s %d\n", command, strlen(token));
			// if the token is one of the commands, then do their works
			// if the token isn't one of the valid commands, report error
			// since commands are always gone first 
			command = token; 
			if (strcmp(command, "ls") == 0) {
				line_cp2 = line_cp;
				
				token = (line_cp2, delimiter, &line_cp2);
				printf("%s\n", token);
				if (strcmp(token, ";") != 0) {
					printf("Error! Unsupported parameters for command: %s\n", command);
					exit(0);
				}
				listDir();
			}
			else if (strcmp(command, "pwd") == 0) {
				showCurrentDir();
			}
			else if (strcmp(command, "mkdir") == 0) {
				printf("it works\n");
			}
			else if (strcmp(command, "cd") == 0) {

			}
			else if (strcmp(command, "cp") == 0) {

			}
			else if (strcmp(command, "mv") == 0) {

			}
			else if (strcmp(command, "rm") == 0) {

			}
			else if (strcmp(command, "ls") == 0) {

			}
			else {
				printf("Error! Unrecognized command: %s\n", command);
			}
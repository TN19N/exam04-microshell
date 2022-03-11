/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   microshell.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mannouao <mannouao@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/03/07 16:25:10 by mannouao          #+#    #+#             */
/*   Updated: 2022/03/07 20:19:59 by mannouao         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include <unistd.h>
# include <stdlib.h>
# include <string.h>
# include <stdio.h>

# define READ 0 // for write end in pipe
# define WRITE 1 // for read end in pipe

# define PIPE 1 // if the cmd followed or preceded by PIPE
# define NORMAL 2 // if the cmd followed or preceded ";" or nathing

typedef struct s_cmd
{
	char	**args; // [0] = {cmd path} [1] = {arg_1} [2] = {arg_2} ....
	int 	type; // followed by "|" or ";" or nothing
	int 	last_type; // preceded by "|" or ";" or nothing == what last cmd followed by 
}				t_cmd;

int ft_strlen(char *str)
{
	int i;

	i = 0;
	while (str[i])
		i++;
	return (i);
}

void	ft_printf(char *str)
{
	int i;

	i = 0;
	while (str[i])
	{
		write (STDERR_FILENO, &str[i], 1);
		i++;
	}
}

char *ft_strdup(char *str)
{
	int		i;
	int		len;
	char	*new_str;

	i = 0;
	len = ft_strlen(str);
	new_str = malloc(sizeof(char) * len);
	if (!new_str)
	{
		ft_printf("error: fatal\n");
		exit(EXIT_FAILURE);
	}
	while (str[i])
	{
		new_str[i] = str[i];
		i++;
	}
	new_str[i] = '\0';
	return (new_str);
}

// set cmds types and save there args
void	init_cmd(t_cmd *cmds, char **av, int index, int i, int j, int type)
{
	int x;

	x = 0;
	cmds[index].type = type;
	// first cmd 100% last_type == NORMAL
	if (index == 0)
		cmds[index].last_type = NORMAL;
	else
	{
		// last type for these cmd is the type of cmd befor hem
		cmds[index].last_type = cmds[index - 1].type;
	}
	cmds[index].args = malloc(sizeof(char *) * ((i - j) + 1));
	if (!cmds[index].args)
	{
		ft_printf("error: fatal\n");
		exit(EXIT_FAILURE);
	}
	while (j < i)
	{
		// store all args i need
		cmds[index].args[x] = ft_strdup(av[j]);
		x++;
		j++;
	}
	// of cours NULL at the end
	cmds[index].args[x] = NULL;
}

// cd function
void	ft_cd(t_cmd *cmds)
{
	int i;

	i = 0;
	while (cmds->args[i])
		i++;
	// we just accept cd with one argumont no less no more
	if (i != 2)
	{
		ft_printf("error: cd: bad arguments\n");
		return ;
	}
	if (chdir(cmds->args[1]) == -1)
	{
		// the derectory dont exist ???
		ft_printf("error: cd: cannot change directory to ");
		ft_printf(cmds->args[1]);
		ft_printf("\n");
		return ;
	}
	// no exit error , we assced just to print the errors :)
	// HINT : these a programe so cd dont have effect becouse the main proccess well finish any way :(
}

int main(int ac, char **av, char **env)
{
	int		number_of_pipes;
	int 	number_of_cmds;
	int		**pipes;
	t_cmd	*cmds;
	int		i;
	int		j;
	int		index;
	int		pid;

	i = 1;
	j = 1;
	index = 0;
	number_of_cmds = 1;
	number_of_pipes = 0;
	if (ac < 2)
		return (EXIT_FAILURE);
	// count number of cmds and pipes
	while (i < ac)
	{
		if (!strcmp(av[i], "|") || !strcmp(av[i], ";"))
		{
			number_of_cmds++;
			if (!strcmp(av[i], "|"))
				number_of_pipes++;
		}
		i++;
	}
	cmds = malloc(sizeof(t_cmd) * number_of_cmds);
	if (!cmds)
	{
		ft_printf("error: fatal\n");
		return (EXIT_FAILURE);
	}
	i = 0;
	// alloc all the pipes i need
	if (number_of_pipes > 0)
	{
		pipes = malloc(sizeof(int *) * number_of_pipes);
		if (!pipes)
		{
			ft_printf("error: fatal\n");
			return (EXIT_FAILURE);
		}
	}
	while (i < number_of_pipes)
	{
		pipes[i] = malloc(sizeof(int) * number_of_pipes);
		if (!pipes[i])
		{
			ft_printf("error: fatal\n");
			return (EXIT_FAILURE);
		}
		i++;
	}
	// start parcing the args
	i = 1;
	while (i < ac)
	{
		if (!strcmp(av[i], "|"))
		{
			init_cmd(cmds, av, index, i, j, PIPE);
			j = i + 1;
			index++;
		}
		else if (!strcmp(av[i], ";"))
		{
			init_cmd(cmds, av, index, i, j, NORMAL);
			j = i + 1;
			index++;
		}
		i++;
	}
	// for last one or the first one if i have just one cmd
	init_cmd(cmds, av, index, i, j, NORMAL);
	// *************** testing **************************
	// i = 0;
	// while (i < number_of_cmds)
	// {
	// 	printf("type = (%d)\n",cmds[i].type);
	// 	printf("last_type = (%d)\n",cmds[i].last_type);
	// 	j = 0;
	// 	while (cmds[i].args[j])
	// 	{
	// 		printf("(%s)\t", cmds[i].args[j]);
	// 		j++;
	// 	}
	// 	printf("\n");
	// 	i++;
	// }
	// **************************************************
	index = 0;
	j = 0;
	i = 0;
	while (i < number_of_cmds)
	{
		// init pipe just if there pipe in head
		if (cmds[i].type == PIPE)
		{
			if (pipe(pipes[index]) == -1)
			{
				ft_printf("error: fatal\n");
				return (EXIT_FAILURE);
			}
		}
		// if we whant te execute a NORMAL cmd we shod make shore that all cmds befor hem are done
		if (cmds[i].type != PIPE && cmds[i].type)
		{
			j = i;
			while (j >= 0)
			{
				if (cmds[j].type == PIPE || cmds[j].last_type == PIPE)
					waitpid(-1, NULL, 0);
				j--;
			}
		}
		// execut cd cmd in main procces if the cmd not ner pipe 
		if (cmds[i].type != PIPE && cmds[i].last_type != PIPE && !strcmp(cmds[i].args[0], "cd"))
		{
			// call cd function
			ft_cd(&cmds[i]);
		}
		else
		{
			// else use execve in child proccess
			pid = fork();
			if (pid == -1)
			{
				ft_printf("error: fatal\n");
				return (EXIT_FAILURE);
			}
			else if (pid == 0)
			{
				if (cmds[i].type == PIPE)
				{
					// dup out put to write end of the pipe
					if (dup2(pipes[index][WRITE], STDOUT_FILENO) == -1)
					{
						ft_printf("error: fatal\n");
						exit(EXIT_FAILURE);
					}
				}
				if (cmds[i].last_type == PIPE)
				{
					// dup in put to read end of the pipe (exactly the last pipe)
					if (dup2(pipes[index - 1][READ], STDIN_FILENO) == -1)
					{
						ft_printf("error: fatal\n");
						exit(EXIT_FAILURE);
					}
				}
				if (execve(cmds[i].args[0], cmds[i].args, env) == -1)
				{
					ft_printf("error: cannot execute ");
					ft_printf(cmds[i].args[0]);
					ft_printf("\n");
					exit(EXIT_FAILURE);
				}
			}
			if (cmds[i].type == PIPE)
			{
				// if the cmd type is PIPE thats mean that we dont need WRITE becous that cmd is 
				// the only one that he shod write to it
				close(pipes[index][WRITE]);
			}
			if (cmds[i].last_type == PIPE)
			{
				// if the cmd last type is PIPE like above its the only one ho can read from it 
				// so we dont need it any more
				close(pipes[index - 1][READ]);
			}
			if (cmds[i].type == PIPE || cmds[i].last_type == PIPE)
			{
				// incrimont index , just if there is a pipe
				index++;
			}
			else
			{
				// if these cmd is NORMAL cmd we shod wait for hem ex(";" sleep 10 ";")
				waitpid(pid, NULL, 0);
			}
		}
		i++;
	}
	// if the last cmd is pipe type we shold whait for hem :)
	while (i >= 0)
	{
		if (cmds[i].type == PIPE || cmds[i].last_type == PIPE)
			wait(NULL);
		i--;
	}
	return (EXIT_SUCCESS);
}
// max opened file descriptors is 5 ((0 . 1 . 2) for in-out-err (3 . 4 . 5) for pipes ends)

// dont forget we accept /bin/ls not ls we dont work with PATH env ver :)

//exam finish easy

// rdo lbal hadi /bin/ls ";" ";" ";" /bin/ls khs tkon bhal zsh mach bash

// RIP ⚰️ ENGLISH :(

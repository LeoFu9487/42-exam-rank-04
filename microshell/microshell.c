// start from 15h
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>

typedef struct s_list
{
	char *str; // no need to free
	struct s_list *next;
	struct s_list *prev;
}t_list;

t_list	*command = NULL;
char	**env = NULL;
int		pipe_exist = 0;
int		fd[2][2] = {{0,1},{0,1}};

void command_push_back(char *str)
{
	t_list *_new = malloc(1 * sizeof(t_list));
	_new->next = NULL;
	_new->str = str;
	_new->prev = NULL;
	if (command == NULL)
	{
		command = _new;
	}
	else
	{
		t_list *temp = command;
		while (temp->next) temp = temp->next;
		temp->next = _new;
		_new->prev = temp;
	}
}

void command_pop_front()
{
	if (command)
	{
		void *ptr = command;
		command = command->next;
		free(ptr);
		if (command)
			command->prev = NULL;
	}
}

void command_clear()
{
	while (command)
	{
		command_pop_front();
	}
}

int ft_strlen(char *str)
{
	if (!str)
		return (0);
	int i = 0;
	while (str[i])
		++i;
	return (i);
}

void ft_putstr_fd(char *str, int fd)
{
	write(fd, str, ft_strlen(str));
}

void run_command() // in parent process : free command, change fd[2][2] back, pipe_exist = 0
{

}

int main(int ac, char **av, char **_env)
{
	env = _env;
	for (int i = 0 ; i < ac ; ++i)
	{
		if (strcmp("|", av[i]) == 0)
		{
			if (!pipe_exist)
			{

			}
			else
			{

			}
		}
		else if (strcmp(";", av[i]) == 0)
			run_command();
		else
			command_push_back(av[i]);
	}
	run_command();
}

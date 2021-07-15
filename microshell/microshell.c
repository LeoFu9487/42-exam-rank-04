//start from 00:26
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>

char **env;

int	ft_strlen(char *str)
{
	if (!str) return (0);
	int i = 0;
	while(str[i]) ++i;
	return (i);
}

void	ft_putstr_fd(char *str, int fd)
{
	write(fd, str, ft_strlen(str));
}

typedef struct s_list
{
	char *str;
	struct s_list	*next;
}t_list;

t_list	*cmd = NULL;

void	cmd_push_back(char *str)
{
	t_list *_new = malloc(sizeof(t_list));
	_new->str = str;
	_new->next = NULL;
	if (cmd == NULL)
	{
		cmd = _new;
		return ;
	}
	t_list *tmp = cmd;
	while (tmp->next)
		tmp = tmp->next;
	tmp->next = _new;
}

void	cmd_pop_front()
{
	if (cmd)
	{
		void *ptr = cmd;
		cmd = cmd->next;
		free(ptr);
	}
}

void	cmd_clear()
{
	while (cmd)
	{
		cmd_pop_front();
	}
}

void	exit_fatal()
{
	if (cmd)
		cmd_clear();
	ft_putstr_fd("error: fatal\n", 2);
	exit(EXIT_FAILURE);
}

int	fd[2] = {0,1};

void	cd() // remember to free
{
	char *str[2];
	if (!cmd)
	{
		ft_putstr_fd("error: cd: bad arguments\n", 2);
		return ;
	}
	str[0] = cmd->str;
	cmd_pop_front();
	if (!cmd)
	{
		ft_putstr_fd("error: cd: bad arguments\n", 2);
		return ;
	}
	str[1] = cmd->str;
	cmd_pop_front();
	if (cmd)
	{
		ft_putstr_fd("error: cd: bad arguments\n", 2);
		return ;
	}
	if (chdir(str[1]) < 0)
	{
		ft_putstr_fd("error: cd: cannot change directory to ", 2);
		ft_putstr_fd(str[1], 2);
		ft_putstr_fd("\n", 2);
		return ;
	}
}

void	run_child() // remember to free
{
	for (int i = 0; i  < 2 ; ++i)
	{
		if (fd[i] != i)
		{
			dup2(fd[i], i);
			close(fd[i]);
		}
	}
	int	cnt = 0;
	t_list	*tmp = cmd;
	while (tmp)
	{
		++cnt;
		tmp = tmp->next;
	}
	char **args = malloc((1 + cnt) * sizeof(char *));
	args[cnt] = 0;
	for (int i = 0 ; i < cnt ; ++i)
	{
		args[i] = cmd->str;
		cmd_pop_front();
	}
	execve(args[0], args, env);
	ft_putstr_fd("error: cannot execute ", 2);
	ft_putstr_fd(args[0], 2);
	ft_putstr_fd("\n", 2);
	free(args);
	exit(EXIT_FAILURE);
}

void	run_cmd() // remember to free
{
	if (strcmp("cd", cmd->str) == 0)
	{
		cd();
		return ;
	}
	if (!cmd) return ;
	pid_t pid = fork();
	if (pid < 0)
		exit_fatal();
	else if (pid) // parent
	{
		waitpid(pid, NULL, 0);
		cmd_clear();
		if (fd[0] != 0)
			close(fd[0]);
		fd[0] = 0;
		fd[1] = 1;
	}
	else // child
		run_child();
}

int	main(int ac, char **av, char **_env)
{
	env = _env;
	for (int i = 1 ; i < ac ; ++i)
	{
		if (strcmp("|", av[i]) == 0)
		{
			int	pipefd[2];
			if (pipe(pipefd) < 0)
				exit_fatal();
			pid_t	pid = fork();
			if (pid < 0)
				exit_fatal();
			else if (pid) // parent
			{
				close(pipefd[1]);
				cmd_clear();
				waitpid(pid, NULL, 0);
				if (fd[0] != 0)
					close(fd[0]);
				fd[0] = pipefd[0];
			}
			else // child
			{
				close(pipefd[0]);
				fd[1] = pipefd[1];
				run_child();
			}
			
		}
		else if (strcmp(";", av[i]) == 0)
		{
			if (cmd)
				run_cmd();
		}
		else
			cmd_push_back(av[i]);
	}
	if (cmd)
		run_cmd();
}

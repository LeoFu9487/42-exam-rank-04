//start from 13h40
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>

char	**env = NULL;
int		ft_strlen(char *str)
{
	if (!str) return 0;
	int n = 0;
	while (str[n]) ++n;
	return n;
}

void	ft_putstr_fd(char *str, int fd)
{
	write(fd, str, ft_strlen(str));
}

typedef struct	s_list
{
	char			*str;
	struct s_list	*next;
}t_list;

t_list	*cmd = NULL;

void	cmd_pop_front()
{
	if (!cmd) return ;
	void	*ptr = cmd;
	cmd = cmd->next;
	free(ptr);
}

void	cmd_clear()
{
	while (cmd)
	{
		cmd_pop_front();
	}
}

void	exit_fatal() // remember to free
{
	if (cmd) cmd_clear();
	ft_putstr_fd("error: fatal\n", 2);
	exit(EXIT_FAILURE);
}

void	cmd_push_back(char *str)
{
	t_list	*new = malloc(sizeof(t_list));
	if (!new)
		exit_fatal();
	new->str = str;
	new->next = NULL;
	if (!cmd) cmd = new;
	else
	{
		while (cmd->next) cmd = cmd->next;
		cmd->next = new;
	}
}

int	fd[2] = {0, 1};

void	cd() // no need to exit, need to free
{
	int	cnt = 0;
	t_list	*iter = cmd;
	while (iter)
	{
		++cnt;
		iter = iter->next;
	}
	if (cnt != 2)
	{
		cmd_clear();
		ft_putstr_fd( "error: cd: bad arguments\n", 2);
		return ;
	}
	if (chdir(cmd->next->str) < 0)
	{
		ft_putstr_fd("error: cd: cannot change directory to ", 2);
		ft_putstr_fd(cmd->next->str, 2);
		ft_putstr_fd("\n", 2);
	}
	cmd_clear();
	return ;
}

void	run_child() // remember to free and to change the fd
{
	if (!cmd) return ;
	for (int i = 0 ; i < 2 ; ++i)
	{
		if (fd[i] != i)
		{
			dup2(fd[i], i);
			close(fd[i]);
		}
	}
	int		cnt = 0;
	t_list	*iter = cmd;
	while (iter)
	{
		++cnt;
		iter = iter->next;
	}
	char	**args;
	args = malloc((1 + cnt) * sizeof(char *));
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

void	run_cmd() // remember to free and the fd (need to close ?)
{
	if (cmd == NULL) return ;
	if (strcmp(cmd->str, "cd") == 0)
	{
		cd();
		return ;
	}
	pid_t	pid = fork();
	if (pid < 0)
		exit_fatal();
	else if (pid) // parent
	{
		cmd_clear();
		if (fd[0] != 0) close(fd[0]);
		fd[0] = 0;
		waitpid(pid, NULL, 0);
	}
	else // child
	{
		run_child();
	}
}

int	main(int ac, char **av, char **_env)
{
	env = _env;
	for (int i = 1 ; i < ac ; ++i)
	{
		if (strcmp(av[i], "|") == 0)
		{
			int	pipefd[2];
			if (pipe(pipefd) < 0)
				exit_fatal();
			pid_t	pid = fork();
			if (pid < 0) exit_fatal();
			else if (pid) // parent
			{
				close(pipefd[1]);
				cmd_clear();
				if (fd[0] != 0)
					close(fd[0]);
				fd[0] = pipefd[0];
				waitpid(pid, NULL, 0);
			}
			else // child
			{
				close(pipefd[0]);
				fd[1] = pipefd[1];
				run_child();
			}
		}
		else if (strcmp(av[i], ";") == 0)
		{
			run_cmd();
		}
		else
		{
			cmd_push_back(av[i]);
		}
	}
	if (cmd)
		run_cmd();
}

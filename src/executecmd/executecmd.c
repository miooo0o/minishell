/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   executecmd.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: minakim <minakim@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/15 14:06:17 by minakim           #+#    #+#             */
/*   Updated: 2023/10/18 15:03:56 by minakim          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int		child(t_sent *cmd, t_deque *deque, int old_fd[2], int fd[2]);
int		ft_execvp(t_sent *cmd);
void	parent(t_sent *cmd, t_deque *deque, int old_fd[2], int fd[2]);
int		wait_child(t_deque *deque, int old_fd[2]);

/// @note 기본적인 멀티 파이프는 작동하지만 아직 builtin과 재대로 연결되지 않았습니다.
/// TODO : memory leak 체크가 하나도 되어 있지 않으므로 전체적인 구조 수정 이후 디버깅 필요

typedef struct s_ctx
{
	t_sent	*cmd;
	int		old_fd[2];
	int		fd[2];
	int		pids[MAX_PIPES];
	int		i;
}				t_ctx;

static t_ctx	*ctx(void)
{
	static t_ctx	this;
	static int		is_init;

	if (is_init)
		return (&this);
	is_init = TRUE;
	this.cmd = NULL;
	this.i = 0;
	return (&this);
}

int	executecmd(t_deque *deque)
{
/// TODO : 현재 var가 많고 앞으로 더 늘어날 미래만 보이므로 struct 만들도록 합시다!
	t_sent	*cmd;
	int		pid;
	int		fd[2];
	int		old_fd[2];
	int		i;

	i = 0;
	while (deque->size > 0 && i < MAX_PIPES)
	{
		cmd = deque_pop_back(deque);
/// TODO : builtin중 parent에서 작동해야하는 exit, export, unset, cd 따로 작동하도록 새로 함수를 만들어야합니다.
/// @note child에서 builtin이 작동하고 있기 때문에 지금 exit는 완전히 작동하지 않습니다!
/// 위의 아이디어에서 확실히 생각해야할 부분 : pipe연결, wait 여부
		if (cmd->next && cmd->output_flag == PIPE_FLAG)
			pipe(fd);
		pid = fork();
		if (check_pid(pid))
			/// need free ?
			return (-1);
		else if (pid == 0) /// child process
			return (child(cmd, deque, old_fd, fd));
		deque->pids[i++] = pid;
		parent(cmd, deque, old_fd, fd);
	}
	return (wait_child(deque, old_fd));
}

int	ft_execvp(t_sent *cmd)
{
	char	**menvp;
	char	*path;
	int		res;

	res = 0;
	menvp = dll_to_envp(ms_env());
	path = ms_find_path(cmd->tokens[0]);
	if (cmd->output_flag == STDERR_FILENO)
	{
		ms_error(cmd->output_argv);
		return (ft_free_check(path, menvp, 1));
	}
	printf("execvp %d\n", getpid());
/// FIXME : For builtin, path == NULL. need to think about where "check_path" should be.
/// FIXME : When running builtin, many fatal errors (SIGs) occur.
	if (is_built_in(cmd))
	{
		printf("yes. builtin\n");
		res = dispatchcmd_wrapper(cmd);
		return (res);
	}
	else
	{
		if (check_path(path, cmd->tokens[0]))
			return (ft_free_check(path, menvp, 1));
		if (execute_node(cmd, menvp, path) < 0)
			return (ft_free_check(path, menvp, -1));
	}
	return (ft_free_check(path, menvp, 0));
}

int	wait_child(t_deque *deque, int old_fd[2])
{
	int	status;
	int	res;
	int	i;

	status = 0;
	res = 0;
	i = -1;
	while (++i < deque->saved_size + 1)
		waitpid(deque->pids[i], &status, 0);
	if (deque->saved_size)
	{
		close(old_fd[0]);
		close(old_fd[1]);
	}
	if (WIFSIGNALED(status) && ms_env()->g_exit != 130)
		res = WTERMSIG(status);
	else if (WIFEXITED(status) && ms_env()->g_exit != 130)
		res = WEXITSTATUS(status);
	else if (WIFSTOPPED(status) && ms_env()->g_exit != 130)
		res = 1;
	return (res);
}

int	child(t_sent *cmd, t_deque *deque, int old_fd[2], int fd[2])
{
	int	res;

	res = 0;
	if (run_by_flag(cmd, INPUT) < 0)
		return (-1);
	if (run_by_flag(cmd, OUTPUT) < 0)
		return (-1);
	if (deque->saved_size > deque->size)
	{
		dup2(old_fd[0], STDIN_FILENO);
		close(old_fd[0]);
		close(old_fd[1]);
	}
	if (deque->size > 0)
	{
		close(fd[0]);
		dup2(fd[1], STDOUT_FILENO);
		close(fd[1]);
	}
	printf("child %d\n", getpid());
	res = ft_execvp(cmd);
	return (res);
}

void	parent(t_sent *cmd, t_deque *deque, int old_fd[2], int fd[2])
{
	printf("parent %d\n", getpid());
	if (deque->saved_size > deque->size)
	{
		close(old_fd[0]);
		close(old_fd[1]);
	}
	if (cmd->next)
	{
		old_fd[1] = fd[1];
		old_fd[0] = fd[0];
	}
}

int	execute_node(t_sent *node, char *menvp[], char *path)
{
	size_t	tmp_size;
	char	**tmp;

	if (node->tokens[0] == NULL || path == NULL)
		return (-1);
	if (node->tokens[0][0] == '/')
	{
		tmp_size = ms_split_size(node->tokens[0], '/');
		tmp = (char **)malloc(sizeof(char *) * (tmp_size + 1));
		tmp = ms_split_process(node->tokens[0], '/', tmp, 0);
		node->tokens[0] = ft_strdup(tmp[tmp_size - 1]);
		ft_free_2d(tmp);
	}
	execve(path, node->tokens, menvp);
	ms_error("Failed to execute command\n");
	return (-1);
}
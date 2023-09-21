/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   executecmd_handler.c                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: minakim <minakim@student.42berlin.de>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/09/18 18:39:49 by minakim           #+#    #+#             */
/*   Updated: 2023/09/21 14:42:37 by minakim          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

typedef struct s_cmd
{
	char	*cmd_name;
	void	(*cmd_func)(t_sent *node, t_elst *lst);
}				t_cmd;

int	dispatchcmd(t_sent *node, t_elst *lst)
{
	static t_cmd	cmd_table[] = {
	{"cd", ft_cd},
	{"echo", ft_echo},
	{"pwd", ft_pwd},
	{"env", ft_env},
	{"unset", ft_unset},
	{"export", ft_export},
	{NULL, NULL}
	};
	int				i;

	i = -1;
	while (cmd_table[++i].cmd_name)
	{
		if (ft_strequ(node->tokens[0], cmd_table[i].cmd_name))
		{
			cmd_table[i].cmd_func(node, lst);
			return (1);
		}
	}
	return (0);
}

typedef struct s_exe {
	int		flag;
	t_mode	type;
	void	(*cmd_func)(t_sent *node, t_elst *lst);
}			t_exe;

static int	compare_flags(t_sent *cmd, int current_flag, t_mode flag_type)
{
	if (flag_type == INPUT)
		return (cmd->input_flag == current_flag);
	else if (flag_type == OUTPUT)
		return (cmd->output_flag == current_flag);
	return (0);
}

void	run_by_flag(t_sent *cmd, t_elst *lst, t_mode flag)
{
	static t_exe	exe_f[] = {
	{HDOC_FLAG, INPUT, flag_heredoc},
	{REDI_RD_FLAG, INPUT, flag_redi_read},
	{REDI_WR_APPEND_FLAG, OUTPUT, flag_redi_append},
	{REDI_WR_TRUNC_FLAG, OUTPUT, flag_redi_trunc},
	{-1, NONE, NULL}
	};
	int				i;

	i = 0;
	while (exe_f[i].cmd_func != NULL)
	{
		if (flag == exe_f[i].type && compare_flags(cmd, exe_f[i].flag, flag))
		{
			exe_f[i].cmd_func(cmd, lst);
			break ;
		}
		i++;
	}
}

/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_exit.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sanghupa <sanghupa@student.42berlin.de>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/04 18:50:24 by minakim           #+#    #+#             */
/*   Updated: 2023/11/01 19:32:22 by minakim          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static int	ft_atoi_exit(const char *s)
{
	int	symbol;
	int	num;
	int	i;

	i = 0;
	num = 0;
	symbol = 1;
	while (ft_isspace(s[i]))
		i++;
	if (s[i] == '-')
		symbol = -1;
	if (s[i] == '-' || s[i] == '+')
		i++;
	while (s[i] != '\0')
	{
		if (ft_isdigit(s[i]))
			num = (num * 10) + (s[i] - '0');
		else
			return (2);
		i++;
	}
	return (symbol * num);
}

void	ft_exit(t_sent *node, t_elst *lst)
{
	if (node->tokens_len == 1)
	{
		ft_putstr_fd("exit\n", STDOUT_FILENO);
		lst->g_exit = 0;
	}
	if (node->tokens_len == 2)
	{
		lst->g_exit = ft_atoi_exit(node->tokens[1]);
		ft_putstr_fd("exit\n", STDOUT_FILENO);
	}
	else if (node->tokens_len > 2)
	{
		ft_putstr_fd("exit: too many arguments\n", STDOUT_FILENO);
		lst->g_exit = 1;
	}
}

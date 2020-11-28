/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: marvin <marvin@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/11/16 15:16:03 by marvin            #+#    #+#             */
/*   Updated: 2020/11/16 15:16:03 by marvin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static int		set_fd(char *str)
{
	int		fd;
	int		append;
	char	c;
	int		len;

	fd = 1;
	while (*str && *str != '>')
		str++;
	if (*str == '>')
	{
		str++;
		append = (*str == '>') ? 1 : 0;
		skip_spaces(&str);
		if (append)
		{
			str++;
			skip_spaces(&str);
			fd = open(str, O_RDWR | O_CREAT | O_APPEND, 0666);
			while (len = read(fd, &c, 1))
				if (len == -1)
				{
					write(1, "Couldn't read file\n", 19);
					break;
				}
		}
		else
			fd = open(str, O_RDWR | O_CREAT | O_TRUNC, 0666);
	}
	if (fd < 0)
		write(1, "Couldn't open file\n", 19);
	return (fd);
}

static int		move_next(char **str)
{
	while (**str && **str != ';')
		(*str)++;
	if (**str)
	{
		(*str)++;
		return (1);
	}
	return (0);
}

static void		check_env(char **str, char **envp)
{
	int		i;
	int		len;
	char	*cpy;
	char	*start;
	char	*var;

	if (*str && **str)
	{
		cpy = ft_strdup(*str);
		start = cpy;
		skip_spaces(&cpy);
		if (*cpy == '$')
		{
			cpy++;
			len = ft_strlen_spa(cpy);
			var = ft_strldup(cpy, len);
			free(start);
			cpy = ft_strjoin(var, "=");
			start = cpy;
			free(var);
			i = 0;
			while (envp[i] && ft_memcmp(cpy, envp[i], len + 1))
				i++;
			if (envp[i])
			{
				free(*str);
				*str = ft_strdup(envp[i] + len + 1);
			}
		}
		free(start);
	}
}

char		**check_command(char *str, char **argv, char **envp)
{
	int	fd;
	char *start;

	check_env(&str, envp);
	start = str;
	while (str && *str)
	{
		fd = set_fd(str);
		skip_spaces(&str);
		if (!ft_memcmp(str, "echo ", 5))
			echo_command(envp, str, fd);
		else if (!ft_memcmp(str, "pwd", 4) || !ft_memcmp(str, "pwd ", 4))
			pwd_command(fd);
		else if (!ft_memcmp(str, "cd ", 3))
			cd_command(envp, str);
		else if (!ft_memcmp(str, "env", 4) || !ft_memcmp(str, "env ", 4))
			env_command(envp, fd);
		else if (!ft_memcmp(str, "./", 2) || !ft_memcmp(str, "../", 3) ||
				!ft_memcmp(str, "/", 1))
			bash_command(str, argv, envp);
		else if (!ft_memcmp(str, "export ", 7))
			envp = export_command(str, envp);
		else if (!ft_memcmp(str, "unset ", 6))
			envp = unset_command(str, envp);
		else if (!ft_memcmp(str, "quit", 4) || !ft_memcmp(str, "exit", 4) ||
				!ft_memcmp(str, "close", 5) || !ft_memcmp(str, "q", 1))
			exit_command(start, envp);
		else if (!check_bin(str, envp, fd))
		{
			write(1, "Command \'", 9);
			ft_putstr_fd(str, 1);
			write(1, "\' not found.\n", 13);
		}
		if (fd > 1)
			close(fd);
		move_next(&str);
	}
	if (start)
		free(start);
	return (envp);
}

int			ft_strlen_pipe(char *str)
{
	int i;

	i = 0;
	while (str[i] && str[i] != '|')
		i++;
	return (i);
}

char		**check_pipe(char *str, char **argv, char **envp)
{
	int		fds[2];
	char	*command;
	int		status;
	int		pid;

	if (str && !str[ft_strlen_pipe(str)])
		envp = check_command(str, argv, envp);
	else
	{
		pipe(fds);
		pid = fork();
		if (pid == 0)
		{
			close(fds[0]);
			dup2(fds[1], 1);
			close(fds[1]);
			command = ft_strldup(str, ft_strlen_pipe(str));
			envp = check_command(command, argv, envp);
			exit(0);
		}
		else
		{
			str += ft_strlen_pipe(str) + 1;
			close(fds[1]);
			if (!fork())
			{

				dup2(fds[0], 0);
				close(fds[0]);
				command = ft_strldup(str, ft_strlen_pipe(str));
				envp = check_command(command, argv, envp);
				exit(0);
			}
			else
				close(fds[0]);
		}
		wait(&status);
		wait(&status);
	}
	return (envp);
}
/*
static void		switch_pipes(int *fds_bef, int *fds_aft)
{
	close(fds_bef[0]);
	close(fds_bef[1]);
	fds_bef[0] = fds_aft[0];
	fds_bef[1] = fds_aft[1];
	pipe(fds_aft);
}

char		**check_pipe(char *str, char **argv, char **envp)
{
	int		final;
	int		first;
	int		fds_aft[2];
	int		fds_bef[2];
	int		status;
	char	*command;
	char	*start;

	start = str;
	if (str && !str[ft_strlen_pipe(str)])
		envp = check_command(str, argv, envp);
	else if (str)
	{
		final = 0;
		first = 1;
		pipe(fds_bef);
		pipe(fds_aft);
		while (!final)
		{
			final = (!str[ft_strlen_pipe(str)]) ? 1 : final;
			if (!fork())
			{
				if (!first)
					dup2(fds_bef[0], 0);
				close(fds_bef[0]);
				close(fds_bef[1]);
				if (!final)
					dup2(fds_aft[1], 1);
				close(fds_aft[0]);
				close(fds_aft[1]);
				command = ft_strldup(str, ft_strlen_pipe(str));
				envp = check_command(command, argv, envp); //probablemente haya que hacer algo con envp del hijo
				exit(0);
			}
			wait(&status);
			str += ft_strlen_pipe(str) + 1;
			first = 0;
			switch_pipes(fds_bef, fds_aft);
		}
		if (start)
			free(start);
		close(fds_aft[0]);
		close(fds_aft[1]);
	}
	return (envp);
}
*/
static int	add_char(char **str, char c)
{
	char	*new;
	int		len;
	int		i;

	if (!(*str))
	{
		if (!(*str = ft_calloc(sizeof(char), 2)))
			return (-1);
		(*str)[0] = c;
		return (0);
	}
	len = ft_strlen(*str);
	if (!(new = ft_calloc(sizeof(char), (len + 2))))
		return (-1);
	i = -1;
	while (++i < len)
		new[i] = (*str)[i];
	new[i] = c;
	if (len)
		free(*str);
	*str = new;
	return (0);
}

static char	*relative_path(char *cwd, char **envp)
{
	char	*home;
	char	*path;

	home = get_env(envp, "HOME");
	if (ft_memcmp(cwd, home, ft_strlen(home)))
		path = ft_strdup(cwd);
	else
		path = ft_strjoin("~", cwd + ft_strlen(home));
	return (path);
}

int			main(int argc, char **argv, char **envp)
{
	char	c;
	char	*str;
	char	*cwd;
	char	buff[4097];

	envp = copy_env(envp, 0);
	while (1)
	{
		str = 0;
		cwd = relative_path(getcwd(buff, 4096), envp);
		write(1, "\033[0;32mminishell:\033[0;0m\033[\033[0;34m", 32);
		ft_putstr_fd(cwd, 1);
		write(1, "\033[0;0m$ ", 8);
		free(cwd);
		while (1)
		{
			if (read(1, &c, 1) == 1 && c == '\n')
				break ;
			if (add_char(&str, c))
				return (-1);
		}
		envp = check_pipe(str, argv, envp);
	}
}

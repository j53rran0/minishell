/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   minishell.h                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: marvin <marvin@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/11/16 15:16:49 by marvin            #+#    #+#             */
/*   Updated: 2020/12/08 21:51:41 by marvin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef MINISHELL_H
# define MINISHELL_H

# include "../libft/libft.h"
# include <fcntl.h>
# include <dirent.h>
# include <signal.h>
# include <errno.h>
# include <string.h>
# include <sys/wait.h>
# include <sys/stat.h>

typedef struct		s_data{
	int		argc;
	char	**argv;
	char	**envp;
	char	**export;
	int		ret;
	char	*str;
	char	**cmds;
	int		child;
}					t_data;

void				skip_spaces(char **str);
void				free_matrix(char **matrix);
void				set_args(char **argv, char *str, int argc);
void				child_sig_handler(int sig);
void				bash_command(t_data *param);
void				cd_command(t_data *param);
void				exit_command(t_data *param);
void				parser(t_data *param);
char				*get_env(char **envp, char *env);
char				**copy_env(char **envp, int add);
char				**export_command(t_data *param, int i);
char				**unset_command(t_data *param, int i);
char				**multiple_env(t_data *param, int fd);
char				**copy_args(t_data *param);
char				**check_command(char *str, t_data *param);
int					check_builtins(int fd, t_data *param);
int					check_bin(int fd, t_data *param);
int					check_pipe(int *fds, char *str, t_data *param);
int					count_args(char *str);
int					ft_strlen_token(char *str);
int					ft_strlen_pipe(char *str);
int					ft_strlen_char(char *str, char c);
#endif
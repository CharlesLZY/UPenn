import React, { useState, useEffect } from 'react';
import { useNavigate } from 'react-router-dom';
import { makeStyles } from '@mui/styles';
import {
  Typography, Box, IconButton, Avatar, Menu, MenuItem, Badge,
} from '@mui/material';

import NotificationsIcon from '@mui/icons-material/Notifications';

import * as fetchUser from '../../utils/fetchUser';

const useStyles = makeStyles({
  username: {
    marginTop: 'auto',
    marginBottom: 'auto',
  },
  icon: {
    marginTop: 'auto',
    marginBottom: 'auto',
    marginRight: 20,
  },
});

export default function UserAvatar() {
  const classes = useStyles();
  const navigate = useNavigate();
  const username = sessionStorage.getItem('userID');
  const [anchor, setAnchor] = useState(null);
  const [notificationNumber, setNotificationNumber] = useState(0);
  const handleClick = (e) => {
    setAnchor(e.currentTarget);
  };

  const handleLogout = () => {
    sessionStorage.removeItem('userID');
    navigate('/');
  };

  const handleClickHome = () => {
    navigate('/userpage');
  };

  const handleClickMessage = () => {
    navigate('/chat');
  };

  const handleClose = () => {
    setAnchor(null);
  };

  useEffect(async () => {
    if (username) {
      const n = await fetchUser.fetchUserNotificationNumber(username);
      setNotificationNumber(n);
    }
  }, []);

  return (
    <Box sx={{ display: 'flex' }}>
      <Box className={classes.icon}>
        <Badge badgeContent={notificationNumber} color="secondary">
          <NotificationsIcon
            sx={{
              color: '#FFF5E1',
              '&:hover': {
                color: '#ffffff',
                cursor: 'pointer',
              },
            }}
            size="large"
            onClick={() => navigate('/notification')}
          />
        </Badge>
      </Box>
      <Box className={classes.icon}>
        <IconButton onClick={handleClick}>
          <Avatar />
        </IconButton>
      </Box>
      <Menu
        id="userMenu"
        anchorEl={anchor}
        open={Boolean(anchor)}
        onClose={handleClose}
        // anchorOrigin={{
        //   vertical: 'top',
        //   horizontal: 'right',
        // }}
        // transformOrigin={{
        //   vertical: 'bottom',
        //   horizontal: 'left',
        // }}
      >
        <Box className={classes.username}>
          <Typography
            variant="subtitle1"
            color="secondary"
            align="center"
          >
            {username}
          </Typography>
        </Box>
        <MenuItem onClick={handleClickHome}> Home </MenuItem>
        <MenuItem onClick={handleClickMessage}> Message </MenuItem>
        <MenuItem onClick={handleLogout}> Logout </MenuItem>
      </Menu>
    </Box>
  );
}

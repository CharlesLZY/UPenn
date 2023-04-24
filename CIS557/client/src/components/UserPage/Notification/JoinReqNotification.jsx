import React from 'react';
import { useNavigate } from 'react-router-dom';
import { makeStyles } from '@mui/styles';
import { Typography, Paper, IconButton } from '@mui/material';

import Close from '@mui/icons-material/Close';
import * as fetchUser from '../../../utils/fetchUser';

const useStyles = makeStyles({
  card: {
    width: 700,
    height: 100,
    marginLeft: 'auto',
    marginRight: 'auto',
    marginTop: 10,
  },

  content: {
    display: 'flex',
    justifyContent: 'center',
  },

});

export default function JoinReqNotification({ notification, userID }) {
  const classes = useStyles();
  const navigate = useNavigate();

  const handleClickClose = async () => {
    await fetchUser.deleteNotification(userID, notification.id);
    navigate(0);
  };

  return (
    <Paper className={classes.card}>
      <IconButton sx={{ marginLeft: '660px' }} onClick={handleClickClose}>
        <Close />
      </IconButton>
      <div className={classes.content}>
        <Typography variant="h6">
          Your request to join &thinsp;
        </Typography>
        <Typography variant="h6" color="secondary">
          {notification.from}
          {' '}
&thinsp;
        </Typography>
        <Typography variant="h6">
          was
          {' '}
          {notification.status}
          {' '}
          by admin.
        </Typography>
      </div>
    </Paper>

  );
}

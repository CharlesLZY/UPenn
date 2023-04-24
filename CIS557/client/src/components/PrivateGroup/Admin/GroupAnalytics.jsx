import React from 'react';
import { useNavigate } from 'react-router-dom';
import { makeStyles } from '@mui/styles';
import {
  Paper, Typography, IconButton, Icon,
} from '@mui/material';

import RefreshIcon from '@mui/icons-material/Refresh';

const useStyles = makeStyles({
  bg: {
    width: 250,
    height: 508,
    marginLeft: 10,
    marginTop: 10,
  },

  titleArea: {
    display: 'flex',
  },

  title: {
    paddingTop: 1,
    paddingLeft: 50,
  },
  term: {
    display: 'flex',
    paddingLeft: 10,
    marginTop: 10,
  },

  content: {
    width: 240,
    marginTop: 10,
    marginLeft: 'auto',
    marginRight: 'auto',
  },
});

export default function GroupAnalytics({ group }) {
  const classes = useStyles();
  const navigate = useNavigate();

  return (
    <Paper className={classes.bg}>
      <div className={classes.titleArea}>
        <Typography variant="h6" className={classes.title}>
          Group Statistics
        </Typography>
        <IconButton size="small" sx={{ marginLeft: '18px', marginTop: '1px' }} onClick={() => navigate(0)}>
          <RefreshIcon />
        </IconButton>
      </div>

      <Paper className={classes.content} elevation={0}>
        <div className={classes.term}>
          <Typography variant="subtitle1">
            # Members :
            {' '}
            {group.members.length}
          </Typography>
        </div>
        <div className={classes.term}>
          <Typography variant="subtitle1">
            # Admins :
            {' '}
            {group.admins.length}
          </Typography>
        </div>
        <div className={classes.term}>
          <Typography variant="subtitle1">
            # Posts :
            {' '}
            {group.posts.length}
          </Typography>
        </div>
        <div className={classes.term}>
          <Typography variant="subtitle1">
            # Flags for deletion :
            {' '}
            {group.deletionRequest.length}
          </Typography>
        </div>
        <div className={classes.term}>
          <Typography variant="subtitle1">
            # Deleted Posts :
            {' '}
            {group.deleted_number}
          </Typography>
        </div>
        <div className={classes.term}>
          <Typography variant="subtitle1">
            # Hidden Posts :
            {' '}
            {group.hidden_number}
          </Typography>
        </div>
      </Paper>
    </Paper>
  );
}

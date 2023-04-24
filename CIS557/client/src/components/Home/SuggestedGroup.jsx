import React from 'react';
import { makeStyles } from '@mui/styles';
import { useNavigate } from 'react-router-dom';
import {
  Paper, Typography, IconButton, Avatar,
} from '@mui/material';

import ArrowForwardIos from '@mui/icons-material/ArrowForwardIos';

import Icon from '../../Icon.svg';

const useStyles = makeStyles({
  root: {
    marginLeft: 20,
    marginTop: 10,
    paddingTop: 20,
    paddingBottom: 20,
    width: 350,
  },

  item: {
    marginTop: 10,
    marginLeft: 'auto',
    marginRight: 'auto',
    '&:hover': {
      backgroundColor: '#F5AF64',
    },
    display: 'flex',
    justifyContent: 'space-between',
    height: 50,
    width: 330,
  },
});

export default function GroupBar({ suggestion }) {
  const classes = useStyles();

  const navigate = useNavigate();

  const handleClick = (groupID) => {
    navigate(`/publicgroup/${groupID}`);
  };

  return (

    <Paper className={classes.root}>
      <Typography align="center" variant="h6">
        Suggested for you
      </Typography>
      <Paper className={classes.item} elevation={2}>
        <Avatar src={Icon} alt="Icon" sx={{ width: '60px', marginTop: 'auto', marginBottom: 'auto' }} />
        <Typography align="center" sx={{ marginTop: 'auto', marginBottom: 'auto' }} variant="h6">
          {suggestion}
        </Typography>
        <IconButton
          sx={{
            width: '50px', height: '50px', marginTop: 'auto', marginBottom: 'auto',
          }}
          onClick={() => handleClick(suggestion)}
        >
          <ArrowForwardIos />
        </IconButton>
      </Paper>
    </Paper>
  );
}

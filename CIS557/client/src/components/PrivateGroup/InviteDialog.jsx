import React, { useState } from 'react';
import { useNavigate } from 'react-router-dom';
import { makeStyles } from '@mui/styles';
import {
  IconButton, Paper, Typography, TextField, Avatar,
} from '@mui/material';

import SearchIcon from '@mui/icons-material/Search';
import Send from '@mui/icons-material/Send';

import * as fetchUser from '../../utils/fetchUser';

const useStyles = makeStyles({
  root: {
    display: 'inlined',
    width: 400,
    height: 200,
    textAlign: 'center',
    paddingTop: 40,
    top: '30%',
    right: document.body.clientWidth / 2 - 200,
    bottom: 'auto',
    left: 'auto',
    position: 'fixed',
  },

  notchedOutline: {
    borderColor: '#c2c2c2 !important',
  },

  searchInfo: {
    marginTop: 30,
  },

  searchResult: {
    marginTop: 20,
    paddingLeft: 10,
    paddingTop: 10,
    display: 'flex',
    width: 270,
    height: 50,
    marginLeft: 56,
    '&:hover': {
      backgroundColor: '#F5AF64',
    },
    justifyContent: 'space-between',
  },

});

export default function InviteDialog({ group, userID }) {
  const classes = useStyles();
  const navigate = useNavigate();
  const [input, setInput] = useState('');
  const [existed, setExisted] = useState(false);
  const [nobody, setNobody] = useState(false);
  const [inviteUser, setInviteUser] = useState(null);

  const handleSearch = async () => {
    const { members } = group;
    if (members.indexOf(input) >= 0) {
      setExisted(true);
    } else {
      const result = await fetchUser.fetchUser(input);
      if (result) {
        setInviteUser(input);
      } else {
        setNobody(true);
      }
    }
  };

  const handleInvite = async (inviteUser) => {
    await fetchUser.invite(userID, inviteUser, group.id, 'private');
    navigate(0);
  };

  return (
    <Paper className={classes.root}>
      <div>
        <TextField
          sx={{
            width: '280px',
            marginTop: '10px',
            marginLeft: '35px',
          }}
          size="small"
          variant="outlined"
          placeholder="Search..."
          id="searchMember-inputBar"
          InputProps={{
            classes: {
              notchedOutline: classes.notchedOutline,
            },
          }}
          inputProps={{ maxLength: 30 }}
          onChange={(e) => {
            setInput(e.target.value);
            setExisted(false);
            setNobody(false);
            setInviteUser(null);
          }}
        />
        <IconButton disabled={input === ''} sx={{ marginTop: '9px', color: '#c0c0c0' }} onClick={handleSearch}>
          <SearchIcon />
        </IconButton>
      </div>
      {existed && <div className={classes.searchInfo}><Typography>The user is already a group member.</Typography></div>}
      {nobody && <div className={classes.searchInfo}><Typography>User does not exist.</Typography></div>}
      {inviteUser
                && (
                <Paper className={classes.searchResult}>
                  <Avatar />
                  <Typography sx={{ marginTop: '8px' }}>{inviteUser}</Typography>
                  <IconButton sx={{ marginBottom: '10px' }} onClick={() => handleInvite(inviteUser)}>
                    <Send />
                  </IconButton>
                </Paper>
                )}
    </Paper>
  );
}

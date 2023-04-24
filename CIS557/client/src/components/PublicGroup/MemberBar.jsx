import React, { useState } from 'react';
import { useNavigate } from 'react-router-dom';
import { FixedSizeList } from 'react-window';

import { makeStyles } from '@mui/styles';
import {
  Paper, IconButton, Typography, InputBase, Avatar,
} from '@mui/material';

import Email from '@mui/icons-material/Email';
import Close from '@mui/icons-material/Close';

import * as fetchMessage from '../../utils/fetchMessage';

const useStyles = makeStyles({
  groupMemberList: {
    marginTop: 10,
    marginLeft: 20,
    paddingLeft: 5,
  },

  searchBar: {
    marginTop: 10,
    marginLeft: 20,
  },

  search: {
    marginTop: 10,
    height: 48,
    display: 'flex',
    justifyContent: 'space-between',
  },

  inputBar: {
    width: 230,
    marginLeft: 15,
  },

  memberlist: {
    marginTop: 5,
  },

  searchResult: {
    '&:hover': {
      backgroundColor: '#F5AF64',
    },
    display: 'flex',
    justifyContent: 'space-between',
    height: 50,
    width: 290,
    marginLeft: 5,
  },

  padding: {
    height: 5,
  },

  item: {
    '&:hover': {
      backgroundColor: '#F5AF64',
    },
    display: 'flex',
    justifyContent: 'space-between',
    height: 50,
  },

  itemAvatar: {
    marginTop: 'auto',
    marginBottom: 'auto',
    marginLeft: 10,
  },

  itemContent: {
    marginTop: 'auto',
    marginBottom: 'auto',
  },
});

export default function MemberBar({ group, userID }) {
  const classes = useStyles();
  const navigate = useNavigate();
  const [input, setInput] = useState('');

  const handleReset = () => {
    document.getElementById('searchMember-inputBar').value = '';
    setInput('');
  };

  const handleClickChat = async (userToChat) => {
    const res = await fetchMessage.createChatThread(userID, userToChat);
    if (res === 404) {
      navigate(0);
    } else {
      navigate('/chat');
    }
  };

  const { members } = group;
  const { creator } = group;
  const { admins } = group;

  function renderRow() {
    return (
      members.map((member) => (
        <div key={member} className={classes.item}>
          <Avatar className={classes.itemAvatar} />
          <Typography align="center" sx={{ marginTop: 'auto', marginBottom: 'auto' }} variant="subtitle1">
            {member}
            {member === creator ? ' (Owner)' : ''}
            {admins.indexOf(member) >= 0 && member !== creator ? ' (Admin)' : ''}
          </Typography>
          <IconButton disabled={!(members.indexOf(userID) >= 0) || (member === userID)} sx={{ marginTop: 'auto', marginBottom: 'auto' }} onClick={() => handleClickChat(member)}>
            <Email />
          </IconButton>
        </div>
      ))
    );
  }

  function searchResult() {
    return (
      members.indexOf(input) >= 0
        ? (
          <div>
            <Paper className={classes.searchResult} elevation={2}>
              <Avatar className={classes.itemAvatar} />
              <Typography align="center" sx={{ marginTop: 'auto', marginBottom: 'auto' }} variant="subtitle1">
                {input}
                {input === creator ? ' (Owner)' : ''}
                {admins.indexOf(input) >= 0 && input !== creator ? ' (Admin)' : ''}
              </Typography>
              <IconButton disabled={!(members.indexOf(userID) >= 0) || (input === userID)} sx={{ marginTop: 'auto', marginBottom: 'auto' }} onClick={() => handleClickChat(input)}>
                <Email />
              </IconButton>
            </Paper>
            <div className={classes.padding} />
          </div>
        )
        : (
          <div className={classes.list}>
            <Typography align="center" className={classes.itemContent} variant="subtitle1">
              Not Found
            </Typography>
          </div>
        )
    );
  }

  return (
    <div>
      <Paper className={classes.searchBar}>
        <div className={classes.search}>
          <InputBase
            className={classes.inputBar}
            placeholder="Search..."
            id="searchMember-inputBar"
            inputProps={{ maxLength: 30 }}
            onChange={(e) => setInput(e.target.value)}
          />
          <IconButton onClick={handleReset}>
            <Close />
          </IconButton>
        </div>
        {input !== '' ? searchResult() : <div />}
      </Paper>
      <Paper className={classes.groupMemberList}>
        <FixedSizeList
          height={250}
          width={295}
          itemSize={50}
          itemCount={1}
        >
          {renderRow}
        </FixedSizeList>
      </Paper>

    </div>
  );
}

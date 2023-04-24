import React, { useState } from 'react';
import { useNavigate } from 'react-router-dom';
import { FixedSizeList } from 'react-window';
import { makeStyles } from '@mui/styles';
import {
  Paper, Typography, IconButton, InputBase, Divider,
} from '@mui/material';

import HowToRegIcon from '@mui/icons-material/HowToReg';
import PersonOff from '@mui/icons-material/PersonOff';
import Close from '@mui/icons-material/Close';

import * as fetchGroup from '../../../utils/fetchPrivateGroup';

const useStyles = makeStyles({
  adminBoard: {
    width: 350,
    height: 508,
    marginLeft: 10,
    marginTop: 10,
  },

  memberItem: {
    display: 'flex',
    justifyContent: 'space-between',
    marginTop: 7,
  },

  searchBar: {
    marginTop: 10,
    height: 40,
    display: 'flex',
    justifyContent: 'space-between',
  },

  inputBar: {
    width: 300,
    marginLeft: 15,
  },

  searchResult: {
    paddingLeft: 10,
    paddingRight: 10,
    marginLeft: 'auto',
    marginRight: 'auto',
    marginBottom: 10,
    paddingTop: 5,
    paddingBottom: 5,
    width: 300,
    '&:hover': {
      backgroundColor: '#FFF5E1',
    },
  },

  title: {
    marginTop: 5,
    display: 'flex',
    justifyContent: 'center',
  },

  fixedSizeList: {
    marginLeft: 'auto',
    marginRight: 'auto',
    marginBottom: 20,
    marginTop: 10,
  },

});

export default function AdminBar({ group, userID }) {
  const classes = useStyles();
  const navigate = useNavigate();

  const [input, setInput] = useState('');

  const handleReset = () => {
    document.getElementById('searchMember-inputBar').value = '';
    setInput('');
  };

  const handleRevoke = async (user) => {
    await fetchGroup.revokeAdmin(group.id, user);
    navigate(0);
  };

  const handlePromote = async (user) => {
    await fetchGroup.promoteAdmin(group.id, user);
    navigate(0);
  };

  function renderMember(member) {
    const { creator } = group;
    const { admins } = group;
    return (
      <div className={classes.memberItem}>
        <Typography align="center" sx={{ marginTop: 'auto', marginBottom: 'auto' }} color={admins.indexOf(member) >= 0 ? 'secondary' : 'textPrimary'} variant="subtitle1">
          {member}
          {member === creator ? ' (Owner)' : ''}
          {admins.indexOf(member) >= 0 && member !== creator ? ' (Admin)' : ''}
        </Typography>
        {
                    userID === creator
                      ? admins.indexOf(member) < 0
                        ? <IconButton sx={{ color: '#888888' }} size="small" onClick={() => handlePromote(member)}><HowToRegIcon /></IconButton>
                        : member !== creator
                          ? <IconButton sx={{ color: '#888888' }} size="small" onClick={() => handleRevoke(member)}><PersonOff /></IconButton>
                          : <div />
                      : <div />
                }
      </div>
    );
  }

  function searchResult() {
    return (
      input
        ? group.members.indexOf(input) >= 0
          ? (
            <Paper className={classes.searchResult}>
              {renderMember(input)}
            </Paper>
          )
          : (
            <div className={classes.list}>
              <Typography align="center" sx={{ marginTop: 'auto', marginBottom: 'auto' }} variant="subtitle1">
                Not Found
              </Typography>
            </div>
          )
        : <div />
    );
  }

  function allmembers() {
    return (
      <div>
        <div>
          {group.admins.map((member) => (
            <div key={member}>
              {renderMember(member)}
            </div>
          ))}
        </div>
        <div>
          {group.members.filter((member) => group.admins.indexOf(member) < 0).map((member) => (
            <div key={member}>
              {renderMember(member)}
            </div>
          ))}
        </div>
      </div>
    );
  }

  return (
    <Paper className={classes.adminBoard}>
      <div className={classes.title}>
        <Typography variant="h6">
          Group Members
        </Typography>
      </div>
      <div className={classes.searchBar}>
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
      {searchResult()}
      <Divider />
      <div>
        <FixedSizeList
          height={378}
          width={320}
          itemSize={50}
          itemCount={1}
          className={classes.fixedSizeList}
        >
          {allmembers}
        </FixedSizeList>
      </div>
    </Paper>
  );
}

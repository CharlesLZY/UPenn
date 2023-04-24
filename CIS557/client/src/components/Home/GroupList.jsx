import React from 'react';
import { makeStyles } from '@mui/styles';
import { useNavigate } from 'react-router-dom';
import { Typography, IconButton, Avatar } from '@mui/material';
import { FixedSizeList } from 'react-window';

import ArrowForwardIos from '@mui/icons-material/ArrowForwardIos';

import Icon from '../../Icon.svg';

const useStyles = makeStyles({
  item: {
    marginRight: 'auto',
    '&:hover': {
      backgroundColor: '#F5AF64',
    },
    display: 'flex',
    justifyContent: 'space-between',
    height: 50,
  },
});

export default function GroupBar({ groups }) {
  const classes = useStyles();
  const navigate = useNavigate();

  const handleClick = (groupID) => {
    navigate(`/publicgroup/${groupID}`);
  };

  function renderRow(groupsToShow) {
    if (groupsToShow.length > 0) {
      return (
        groupsToShow.map((group) => (
          <div key={group.id} className={classes.item}>
            <Avatar src={Icon} alt="Icon" sx={{ width: '60px', marginTop: 'auto', marginBottom: 'auto' }} />
            <Typography align="center" sx={{ marginTop: 'auto', marginBottom: 'auto' }} variant="subtitle1">
              {group.id}
            </Typography>
            <IconButton
              sx={{
                width: '50px', height: '50px', marginTop: 'auto', marginBottom: 'auto',
              }}
              onClick={() => handleClick(group.id)}
            >
              <ArrowForwardIos />
            </IconButton>
          </div>
        ))
      );
    }
    return (
      <div />
    );
  }

  return (
    <div className={classes.list}>
      <FixedSizeList
        height={250}
        width={350}
        itemSize={50}
        itemCount={1}
      >
        {() => renderRow(groups)}
      </FixedSizeList>
    </div>
  );
}
